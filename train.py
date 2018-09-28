"""
Code to support training a model.
"""
from typing import NamedTuple, List, Tuple
import torch

from model import Model


class ModelSpecification(NamedTuple):
    """
    Named tuple for specifying a model to be created later.
    """
    inputs: List[int]
    outputs: List[int]
    feature_extractors: List[str]


class HyperParams(NamedTuple):
    """
    Named tuple for storing hyperparameters.
    """
    learning_rate: float = 0.0001
    batch_size: int = 5
    discount_factor: float = 0.99
    gae: float = 1.
    entropy_coef: float = 0.001


class TrainingSession(object):
    """
    Manages a training session for an agent.
    """

    def __init__(
            self,
            model: ModelSpecification,
            hyperparams: HyperParams,
            contexts: int,
            auto_train: bool = True):
        self.contexts = contexts
        self.hidden_states = torch.zeros(contexts, 128)
        self.hyperparams = hyperparams
        self.auto_train = auto_train

        self.rewards = [[] for _ in range(contexts)]
        self.log_probs = [[] for _ in range(contexts)]
        self.values = [[] for _ in range(contexts)]
        self.entropies = [[] for _ in range(contexts)]
        self.latest_observations = [None for _ in range(contexts)]

        self.model = Model(model.inputs, model.outputs,
                           model.feature_extractors)

        self.optimizer = torch.optim.RMSprop(
            self.model.parameters(), lr=hyperparams.learning_rate)

    def get_action(
        self,
        inputs: List[torch.Tensor],
        context: int
    ) -> Tuple[List[int], torch.Tensor]:
        """
        Given an observation, get an action and the value of the observation
        from one of the models being trained.
        """
        self.latest_observations[context] = inputs
        value, probs, log_probs = self.model.act(inputs)

        actions = [x.multinomial(num_samples=1) for x in probs]

        self.values[context].append(value)
        log_prob = [log_probs[i][x] for i, x in enumerate(actions)]
        self.log_probs[context].append(torch.stack(log_prob))
        self.entropies[context].append(
            -(torch.cat(log_probs) * torch.cat(probs)).sum(0, keepdim=True))

        return actions, value

    def give_reward(self, reward: float, context: int):
        """
        Assign a reward to the last action performed.
        """
        if len(self.rewards[context]) != len(self.log_probs[context]) - 1:
            raise Exception(
                f"Rewards and actions out of sync.\n"
                f"{len(self.rewards[context])} rewards and "
                f"{len(self.log_probs[context])} actions have been recorded.")

        self.rewards[context].append(reward)

        if self.auto_train:
            if (min([len(x) for x in self.rewards])
                    >= self.hyperparams.batch_size):
                self.train()

    def train(self, final: bool = False):
        """
        Train on a batch of data.
        """
        total_actor_loss = 0
        total_critic_loss = 0

        for context, log_probs in enumerate(self.log_probs):
            log_probs = torch.stack(log_probs)
            values = self.values[context]
            rewards = self.rewards[context]
            entropies = self.entropies[context]
            latest_observation = self.latest_observations[context]

            # Get values of final timesteps
            if final:
                values.append(torch.Tensor([0]))
            else:
                value, _ = self.model(latest_observation)
                values.append(value)

            critic_loss = 0
            actor_loss = 0
            gae = 0
            real_value = values[-1]

            for i in reversed(range(len(log_probs))):
                real_value = (self.hyperparams.discount_factor * real_value
                              + rewards[i])
                advantage = real_value - values[i]
                critic_loss = critic_loss + 0.5 * advantage.pow(2)

                value_delta = (rewards[i]
                               + self.hyperparams.discount_factor
                               * values[i + 1].data
                               - values[i].data)
                gae = (gae
                       * self.hyperparams.discount_factor
                       * self.hyperparams.gae
                       + value_delta)

                actor_loss = (actor_loss
                              - (log_probs[i] * gae).sum()
                              - entropies[i] * self.hyperparams.entropy_coef)

            total_actor_loss += actor_loss
            total_critic_loss += critic_loss

        loss = total_actor_loss + total_critic_loss

        loss.backward()
        self.optimizer.step()

        self.rewards = [[] for _ in range(self.contexts)]
        self.log_probs = [[] for _ in range(self.contexts)]
        self.values = [[] for _ in range(self.contexts)]
        self.entropies = [[] for _ in range(self.contexts)]
        self.latest_observations = [None for _ in range(self.contexts)]
