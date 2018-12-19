"""
Train
"""
import logging
import time
from typing import NamedTuple, Tuple, List
import numpy as np
import torch
from gym import spaces

from a2c_ppo_acktr.algo import PPO
from a2c_ppo_acktr.storage import RolloutStorage

from .model import CustomPolicy, ModelSpecification
from .utils import RunningMeanStd


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
    normalize_rewards: bool = True


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

        # Keeping track of FPS
        self.fps_tracker = FpsTracker()

        if hyperparams.normalize_rewards:
            self.ret_rms = RunningMeanStd()
            self.returns = np.zeros(contexts)
            self.reward_clip = 10.

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
            obs: List[List[float]]
    ) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Given a set of observations, get a list of actions and the values of
        the observations from the TrainingSession's model.
        """
        obs = torch.Tensor(obs)
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

        self.fps_tracker.next_frame()

        return self.last_actions, self.last_values

    def give_rewards(self, rewards: List[float], dones: List[float]):
        """
        Assign rewards to the last set of actions performed.
        """
        if self.state != 'waiting_for_reward':
            raise ValueError("State should be 'waiting_for_reward', but "
                             f"instead is {self.state}")

        # Normalise rewards
        if self.hyperparams.normalize_rewards:
            self.returns = (self.returns
                            * self.hyperparams.discount_factor
                            + rewards)
            self.ret_rms.update(self.returns)
            rewards = np.clip(rewards / np.sqrt(self.ret_rms.var + 1e-8),
                              -self.reward_clip,
                              self.reward_clip)

        rewards = torch.Tensor(rewards).unsqueeze(1)

        masks = torch.FloatTensor([[0.0] if done else [1.0]
                                   for done in dones])

        self.rollouts.insert(self.last_observations, self.last_hidden_states,
                             self.last_actions, self.last_action_log_probs,
                             self.last_values, rewards, masks)

        # If we've collected a whole batch of data, update
        self.step = (self.step + 1) % self.hyperparams.batch_size
        if self.step == 0:
            self.update()

        self.state = 'waiting_for_observation'

    def save_model(self, path: str):
        """
        Saves the current model to disk.
        """
        torch.save(self.model.state_dict(), path)

    def update(self):
        """
        Update the agent based on the recorded data.
        """
        self.fps_tracker.pause()

        logging.debug("---")
        logging.debug("Training...")

        # Calculate returns for batch
        with torch.no_grad():
            next_value = self.model.get_value(
                self.rollouts.obs[-1],
                self.rollouts.recurrent_hidden_states[-1],
                self.rollouts.masks[-1]).detach()
        self.rollouts.compute_returns(next_value, self.hyperparams.use_gae,
                                      self.hyperparams.discount_factor,
                                      self.hyperparams.gae)

        # Update the agent
        value_loss, action_loss, entropy = self.agent.update(self.rollouts)

        action_dist = self.rollouts.actions.view(-1, 4).mean(dim=0)
        action_dist_str = " ".join(format(x, ".2f") for x in action_dist)

        # Reset the rollout storage
        self.rollouts.after_update()

        update_time = self.fps_tracker.unpause()

        logging.debug("Action distribution: %s", action_dist_str)
        logging.debug("Action loss: %+7.4f", action_loss)
        logging.debug("Value loss: %+8.4f", value_loss)
        logging.debug("Entropy: %11.4f", entropy)
        logging.debug("Update took %.2fs", update_time)
        logging.debug("---")


class FpsTracker:
    """
    Keeps track of the current FPS, and displays it periodically.
    """

    def __init__(self):
        self.frame_counter = 0
        self.last_fps_display_time = None
        self.fps_history = []
        self.paused = False
        self.pause_time = None

    def next_frame(self):
        """
        Marks that a frame has passed.
        If enough time has passed, displays the FPS.
        """
        if self.paused:
            return

        # Initialize last display time
        if self.last_fps_display_time is None:
            self.last_fps_display_time = time.time()

        self.frame_counter += 1

        # If more than a second has passed, display the average FPS and reset
        if time.time() - self.last_fps_display_time > 1:
            self.display_fps()

    def display_fps(self):
        """
        Outputs the FPS to the logger.
        """
        fps = (self.frame_counter
               / (time.time() - self.last_fps_display_time))

        self.fps_history.insert(0, fps)
        if len(self.fps_history) > 10:
            self.fps_history.pop()

        logging.debug("FPS: %.2f", np.mean(self.fps_history))

        self.frame_counter = 0
        self.last_fps_display_time = time.time()

    def pause(self):
        """
        Pause the FPS timer.
        """
        if self.paused:
            return

        self.pause_time = time.time()
        self.paused = True

    def unpause(self):
        """
        Unpause the FPS timer.
        """
        if not self.paused:
            return 0

        self.paused = False
        time_spend_paused = time.time() - self.pause_time

        self.last_fps_display_time += time_spend_paused

        return time_spend_paused
