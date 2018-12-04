"""
Train
"""
from typing import NamedTuple, Tuple
import torch
import numpy as np
from gym import spaces

from a2c_ppo_acktr.algo import PPO
from a2c_ppo_acktr.storage import RolloutStorage

from .model import CustomPolicy, ModelSpecification


class HyperParams(NamedTuple):
    """
    Named tuple for storing hyperparameters.
    """
    learning_rate: float = 0.0001
    batch_size: int = 250
    num_minibatch: int = 32
    epochs: int = 5
    discount_factor: float = 0.99
    use_gae: bool = True
    gae: float = 1.
    critic_coef: float = 0.5
    entropy_coef: float = 0.001
    max_grad_norm: float = 0.5
    clip_factor: float = 0.2
    use_gpu: bool = False


class TrainingSession:
    """
    Manages a training session for an agent.
    """

    def __init__(
            self,
            model: ModelSpecification,
            hyperparams: HyperParams,
            contexts: int
    ):
        torch.set_num_threads(1)
        self.device = torch.device("cuda:0" if hyperparams.use_gpu else "cpu")

        self.hyperparams = hyperparams

        self.model = CustomPolicy(model.inputs, model.outputs, model.recurrent)
        self.model.to(self.device)

        self.agent = PPO(self.model, hyperparams.clip_factor,
                         hyperparams.epochs, hyperparams.num_minibatch,
                         hyperparams.critic_coef, hyperparams.entropy_coef,
                         lr=hyperparams.learning_rate, eps=1e-8,
                         max_grad_norm=hyperparams.max_grad_norm)

        self.rollouts = RolloutStorage(hyperparams.batch_size, contexts,
                                       [model.inputs],
                                       spaces.MultiBinary(model.outputs), 128)

        self.last_observations = None
        self.last_hidden_states = None
        self.last_actions = None
        self.last_action_log_probs = None
        self.last_values = None

        self.step = 0

        # TrainingServer is a state machine
        # It has three states:
        # - wating_for_observation: Waiting to be sent an observation.
        #                           When an observation is received, it will
        #                           return an action and wait for the next
        #                           observation.
        # - waiting_for_reward: Waiting to be sent a reward.
        #                       After sending an action, the agent expects a
        #                       reward.
        #                       Upon receiving a reward, stores it it and waits
        #                       for an observation.
        # - start: We are waiting for an observation, but when we receive it we
        #          also haven't received a reward yet so can't do the normal
        #          processing.
        self.state = 'start'

    def get_actions(
            self,
            obs: torch.Tensor
    ) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Given a set of observations, get a list of actions and the values of
        the observations from the TrainingSession's model.
        """
        if self.state == 'start':
            self.rollouts.obs[0].copy_(obs)
            self.rollouts.to(self.device)
        elif self.state != 'waiting_for_observation':
            raise ValueError("State should be 'start' or "
                             "'waiting_for_observation', but instead is "
                             f"{self.state}")

        self.last_observations = obs

        with torch.no_grad():
            (self.last_values, self.last_actions, self.last_action_log_probs,
             self.last_hidden_states) = self.model.act(
                 self.rollouts.obs[self.step],
                 self.rollouts.recurrent_hidden_states[self.step],
                 self.rollouts.masks[self.step])

        self.state = 'waiting_for_reward'

        return self.last_actions, self.last_values

    def give_rewards(self, rewards: np.ndarray, dones: np.ndarray):
        """
        Assign rewards to the last set of actions performed.
        """
        if self.state != 'waiting_for_reward':
            raise ValueError("State should be 'waiting_for_reward', but "
                             f"instead is {self.state}")

        masks = torch.FloatTensor([[0.0] if done else [1.0]
                                   for done in dones])

        self.rollouts.insert(self.last_observations, self.last_hidden_states,
                             self.last_actions, self.last_action_log_probs,
                             self.last_values, rewards, masks)

        self.step = (self.step + 1) % self.hyperparams.batch_size
        if self.step == 0:
            with torch.no_grad():
                next_value = self.model.get_value(
                    self.rollouts.obs[-1],
                    self.rollouts.recurrent_hidden_states[-1],
                    self.rollouts.masks[-1]).detach()

            self.rollouts.compute_returns(next_value, self.hyperparams.use_gae,
                                          self.hyperparams.discount_factor,
                                          self.hyperparams.gae)

            self.agent.update(self.rollouts)
            self.rollouts.after_update()

        self.state = 'waiting_for_observation'

    def save_model(self, path: str):
        """
        Saves the current model to disk.
        """
        torch.save(self.model.state_dict(), path)
