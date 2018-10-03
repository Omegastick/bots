"""
Train
"""
import logging
from typing import NamedTuple, List, Tuple
import torch
import torch.nn.functional as F
import numpy as np

from .model import Model, ModelSpecification


class HyperParams(NamedTuple):
    """
    Named tuple for storing hyperparameters.
    """
    learning_rate: float = 0.0001
    batch_size: int = 250
    minibatch_length: int = 5
    minibatch_count: int = 50
    epochs: int = 5
    discount_factor: float = 0.99
    gae: float = 1.
    critic_coef: float = 0.5
    entropy_coef: float = 0.001
    max_grad_norm: float = 0.5
    clip_factor: float = 0.2


class TrainingSession:
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
        self.observations = [[] for _ in range(contexts)]
        self.actions = [[] for _ in range(contexts)]

        self.model = Model(model.inputs, model.outputs,
                           model.feature_extractors)

        self.optimizer = torch.optim.RMSprop(
            self.model.parameters(), lr=hyperparams.learning_rate)

    def get_action(
            self,
            inputs: List[torch.Tensor],
            context: int) -> Tuple[List[int], torch.Tensor]:
        """
        Given an observation, get an action and the value of the observation
        from one of the models being trained.
        """
        value, probs, log_probs = self.model.act(inputs)

        actions = [x.multinomial(num_samples=1) for x in probs]

        log_prob = [log_probs[i][x] for i, x in enumerate(actions)]

        self.observations[context].append(inputs)
        self.values[context].append(value.detach())
        self.log_probs[context].append(torch.stack(log_prob).detach())
        self.actions[context].append(torch.stack(actions).detach())

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
        logging.debug("Training")
        for _ in range(self.hyperparams.epochs):
            total_actor_loss = 0
            total_critic_loss = 0
            for context, starting_index in self._get_starting_indexes():
                minibatch_length = self.hyperparams.minibatch_length
                rewards = self.rewards[context][
                    starting_index:starting_index + minibatch_length]
                observations = self.observations[context][
                    starting_index:starting_index + minibatch_length + 1]
                old_log_probs = self.log_probs[context][
                    starting_index:starting_index + minibatch_length]
                actions = self.actions[context][
                    starting_index:starting_index + minibatch_length]

                critic_loss = 0
                actor_loss = 0
                gae = 0
                next_value = self.model.get_value(observations[-1])
                real_value = next_value
                values = []

                for i in reversed(range(minibatch_length)):
                    value, raw_probs = self.model.forward(observations[i])
                    values.append(value)
                    probs = [F.softmax(raw_prob, dim=0)
                             for raw_prob in raw_probs]
                    log_probs = [F.log_softmax(raw_prob, dim=0)
                                 for raw_prob in raw_probs]
                    entropy = -(torch.cat(log_probs)
                                * torch.cat(probs)).sum(0, keepdim=True)

                    # Empirical value
                    real_value = (self.hyperparams.discount_factor * real_value
                                  + rewards[i])
                    # Advantage
                    advantage = real_value - value
                    # Critic loss
                    critic_loss = critic_loss + 0.5 * advantage.pow(2)
                    # Difference between this value and the next value
                    value_delta = (rewards[i]
                                   + self.hyperparams.discount_factor
                                   * next_value.data
                                   - value.data)
                    next_value = value
                    # Generalised Advantage Estimate
                    # When setting the GAE hyperparameter to a low value, the
                    # empirical advantage is ignored and instead the value
                    # delta is used.
                    # This is equivalent to trusting your critic over the
                    # empirical advantage.
                    gae = (gae
                           * self.hyperparams.discount_factor
                           * self.hyperparams.gae
                           + value_delta)
                    # PPO
                    clip = self.hyperparams.clip_factor
                    # This line is pretty confusing to look at. I feel bad for
                    # writing it. I actually considered restructuring the whole
                    # way I do data management to prevent this line.
                    # It selects the probability that corresponds to the
                    # action of the old probability.
                    new_log_probs = torch.stack([
                        log_probs[x][actions[i][x]]
                        for x in range(len(actions[i]))])
                    ratio = torch.exp(new_log_probs - old_log_probs[i])
                    surr_1 = ratio * gae
                    surr_2 = torch.clamp(ratio, 1 - clip, 1 + clip) * gae
                    actor_loss = (-torch.min(surr_1, surr_2).mean()
                                  - entropy * self.hyperparams.entropy_coef)

                total_actor_loss += actor_loss
                total_critic_loss += critic_loss

            loss = total_actor_loss + (total_critic_loss
                                       * self.hyperparams.critic_coef)

            self.optimizer.zero_grad()
            loss.backward()

            torch.nn.utils.clip_grad_norm_(self.model.parameters(),
                                           self.hyperparams.max_grad_norm)

            self.optimizer.step()

        self.rewards = [[] for _ in range(self.contexts)]
        self.log_probs = [[] for _ in range(self.contexts)]
        self.values = [[] for _ in range(self.contexts)]
        self.entropies = [[] for _ in range(self.contexts)]
        self.latest_observations = [None for _ in range(self.contexts)]

    def save_model(self, path: str):
        """
        Saves the current model to disk.
        """
        torch.save(self.model.state_dict(), path)

    def _get_starting_indexes(self) -> List[Tuple[int, int]]:
        """
        Gets the starting index for each minibatch.
        Returns a list of tuples in the form of (context, index)
        Contexts rotate uniformly through all available contexts.
        Indexes are sampled uniformly between 0 and the number of timesteps
        observed for that context minus (minibatch_length + 1).
        """
        indexes = []
        for _ in range(self.hyperparams.minibatch_count):
            for context in range(self.contexts):
                index = np.random.randint(
                    0,
                    (len(self.rewards[context])
                     - self.hyperparams.minibatch_length + 1)
                )
                indexes.append((context, index))

        return indexes
