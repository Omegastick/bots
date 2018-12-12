"""
Agent models
"""
from typing import List, NamedTuple, Tuple
import numpy as np
import torch
import torch.nn as nn
from a2c_ppo_acktr.model import MLPBase
from a2c_ppo_acktr.distributions import Bernoulli

from training_server.utils import RunningMeanStd


class ModelSpecification(NamedTuple):
    """
    Named tuple for specifying a model to be created later.
    """
    inputs: List[int]
    outputs: int
    recurrent: bool = False
    normalize_observations: bool = False


class CustomPolicy(nn.Module):
    """
    The policy portion of a reinforcement learning agent.
    Can be customised in the constructor.
    """

    def __init__(
            self,
            inputs: int,
            outputs: int,
            recurrent=False,
            hidden_size=128,
            normalize_observations=False):
        super().__init__()
        self.base = MLPBase(inputs, recurrent, hidden_size)

        self.normalize_observations = normalize_observations
        if self.normalize_observations:
            self.obs_rms = RunningMeanStd(shape=(inputs,))
            self.obs_clip = 10.

        self.gamma = 0.99
        self.epsilon = 1e-8

        self.dist = Bernoulli(self.base.output_size, outputs)

    @property
    def is_recurrent(self) -> bool:
        """
        Whether or not the policy contains a recurrent cell.
        """
        return self.base.is_recurrent

    @property
    def recurrent_hidden_state_size(self) -> int:
        """
        Size of rnn_hx.
        """
        return self.base.recurrent_hidden_state_size

    def forward(self, inputs, rnn_hxs, masks):
        """
        DO NOT USE.
        Not implemented.
        """
        raise NotImplementedError

    def act(
        self,
        inputs,
        rnn_hxs,
        masks,
        deterministic=False) -> Tuple[torch.Tensor, torch.Tensor,
                                      torch.Tensor, torch.Tensor]:
        """
        Get a value, action, log probabilities for the action and the hidden
        states for the inputs given.
        """
        # Normalise inputs
        if self.normalize_observations:
            self.obs_rms.update(inputs)
            inputs = np.clip(
                ((inputs - self.ob_rms.mean)
                 / np.sqrt(self.ob_rms.var + self.epsilon)),
                -self.obs_clip,
                self.obs_clip)

        value, actor_features, rnn_hxs = self.base(inputs, rnn_hxs, masks)
        dist = self.dist(actor_features)

        if deterministic:
            action = dist.mode()
        else:
            action = dist.sample()

        action_log_probs = dist.log_probs(action)

        return value, action, action_log_probs, rnn_hxs

    def get_value(self, inputs, rnn_hxs, masks):
        """
        Get the value of a state.
        """
        value, _, _ = self.base(inputs, rnn_hxs, masks)
        return value

    def evaluate_actions(
            self,
            inputs,
            rnn_hxs,
            masks,
            action) -> Tuple[torch.Tensor, torch.Tensor,
                             torch.Tensor, torch.Tensor]:
        """
        Given a set of states and actions, evaluate taking those action in
        those states.
        """
        value, actor_features, rnn_hxs = self.base(inputs, rnn_hxs, masks)
        dist = self.dist(actor_features)

        action_log_probs = dist.log_probs(action)
        dist_entropy = dist.entropy().mean()

        return value, action_log_probs, dist_entropy, rnn_hxs
