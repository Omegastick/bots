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
    epochs: int = 5
    discount_factor: float = 0.99
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
            contexts: int,
            auto_train: bool = True):

        if hyperparams.use_gpu:
            torch.set_default_tensor_type('torch.cuda.FloatTensor')

        self.contexts = contexts
        self.hyperparams = hyperparams
        self.auto_train = auto_train

        self.rewards = [[] for _ in range(contexts)]
        self.log_probs = [[] for _ in range(contexts)]
        self.values = [[] for _ in range(contexts)]
        self.observations = [[] for _ in range(contexts)]
        self.actions = [[] for _ in range(contexts)]
        self.masks = [[1] for _ in range(contexts)]

        self.model = Model(model.inputs, model.outputs,
                           model.feature_extractors)

        if hyperparams.use_gpu:
            self.model = self.model.cuda()

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
        with torch.no_grad():
            value, probs, log_probs = self.model.act(inputs)

        actions = [x.multinomial(num_samples=1) for x in probs]

        log_prob = [log_probs[i][x] for i, x in enumerate(actions)]

        self.observations[context].append(inputs)
        self.values[context].append(value.detach())
        self.log_probs[context].append(torch.stack(log_prob).detach())
        self.actions[context].append(torch.stack(actions).detach())

        return actions, value

    def give_reward(self, reward: float, context: int, done: bool = False):
        """
        Assign a reward to the last action performed.
        """
        if len(self.rewards[context]) != len(self.log_probs[context]) - 1:
            raise Exception(
                f"Rewards and actions out of sync.\n"
                f"{len(self.rewards[context])} rewards and "
                f"{len(self.log_probs[context])} actions have been recorded.")

        self.rewards[context].append(reward)
        self.masks[context].append(1 - done)

        if self.auto_train:
            if (min([len(x) for x in self.rewards])
                    >= self.hyperparams.batch_size + 1):
                self.train()

    def train(self):
        """
        Train on a batch of data.
        """
        logging.debug("Training")
        for epoch in range(self.hyperparams.epochs):
            for context, starting_index in self._get_starting_indexes(epoch):
                minibatch_length = self.hyperparams.minibatch_length
                # Get minibatch data from memory.
                rewards, observations, old_log_probs, \
                    actions, masks = self._get_rollout(context, starting_index)

                values, raw_probs = self.model.forward(observations)

                real_value = values[-1]
                critic_loss = 0
                actor_loss = 0
                gae = 0
                # We loop backward to make GAE easier to calculate
                for i in reversed(range(minibatch_length)):
                    probs = [F.softmax(raw_prob[i], dim=0)
                             for raw_prob in raw_probs]
                    log_probs = [F.log_softmax(raw_prob[i], dim=0)
                                 for raw_prob in raw_probs]
                    entropy = -(torch.cat(log_probs)
                                * torch.cat(probs)).sum(0, keepdim=True)

                    # Empirical value
                    real_value = (real_value
                                  * self.hyperparams.discount_factor
                                  * masks[i]
                                  + rewards[i])
                    # Advantage
                    advantage = real_value - values[i]
                    # Critic loss
                    critic_loss = critic_loss + 0.5 * advantage.pow(2)
                    # Difference between this value and the next value
                    value_delta = (rewards[i]
                                   + self.hyperparams.discount_factor
                                   * values[i + 1].detach()
                                   - values[i].detach())
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

                loss = actor_loss + (critic_loss
                                     * self.hyperparams.critic_coef)

                self.optimizer.zero_grad()
                loss.backward()

                torch.nn.utils.clip_grad_norm_(
                    self.model.parameters(), self.hyperparams.max_grad_norm)

                self.optimizer.step()

        self.rewards = [[] for _ in range(self.contexts)]
        self.log_probs = [[] for _ in range(self.contexts)]
        self.values = [[] for _ in range(self.contexts)]
        self.observations = [[] for _ in range(self.contexts)]
        self.actions = [[] for _ in range(self.contexts)]
        self.masks = [[1] for _ in range(self.contexts)]

    def save_model(self, path: str):
        """
        Saves the current model to disk.
        """
        torch.save(self.model.state_dict(), path)

    def _get_starting_indexes(self, epoch: int) -> List[Tuple[int, int]]:
        """
        Gets the starting index for each minibatch.
        Returns a list of tuples in the form of (context, index)
        Contexts rotate uniformly through all available contexts.
        Indexes are sampled uniformly between 0 and the number of timesteps
        observed for that context minus (minibatch_length + 1).
        """
        original_indexes = np.arange(0,
                                     self.hyperparams.batch_size,
                                     self.hyperparams.minibatch_length)
        if epoch % 2 == 1:
            original_indexes = original_indexes[:-1]
        original_indexes = np.random.permutation(original_indexes)
        indexes = []
        for index in original_indexes:
            if epoch % 2 == 1:
                index += self.hyperparams.minibatch_length // 2
            for context in range(self.contexts):
                indexes.append((context, index))
        return indexes

    def _get_rollout(self, context: int, starting_index: int
                     ) -> Tuple[List, List, List, List, List]:
        """
        Gets the minibatch data from the batch.
        """
        minibatch_length = self.hyperparams.minibatch_length
        rewards = self.rewards[context][
            starting_index:starting_index + minibatch_length]
        observations = self.observations[context][
            starting_index:starting_index + minibatch_length + 1]
        # This converts the observations into the right shape for
        # processing them all at once:
        # [
        #    sensor_1: torch.Tensor([t_1, t_2, t_3, ...]),
        #    sensor_2: torch.Tensor([t_1, t_2, t_3, ...])
        # ]
        observations = [
            torch.stack([
                observation[idx] for observation in observations])
            for idx, _ in enumerate(observations[0])]
        old_log_probs = self.log_probs[context][
            starting_index:starting_index + minibatch_length]
        actions = self.actions[context][
            starting_index:starting_index + minibatch_length]
        masks = torch.Tensor(self.masks[context][
            starting_index:starting_index + minibatch_length + 1])
        return (rewards, observations, old_log_probs, actions,
                masks)
