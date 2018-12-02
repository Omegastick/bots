"""
Agent models
"""
from typing import List, Tuple, NamedTuple, Union
import torch
import torch.nn as nn
import torch.nn.functional as F
from a2c_ppo_acktr.model import NNBase
from a2c_ppo_acktr.utils import init
from a2c_ppo_acktr.distributions import Bernoulli


class ModelSpecification(NamedTuple):
    """
    Named tuple for specifying a model to be created later.
    """
    inputs: List[Union[int, List[int]]]
    outputs: List[int]
    feature_extractors: List[str]
    recurrent: bool = True  # Currently deprecated.
    #                         Will be reimplemented eventually.
    kernel_sizes: List[int] = [8, 4, 3]
    kernel_strides: List[int] = [4, 2, 1]


class CustomPolicy(nn.Module):
    """
    The policy portion of a reinforcement learning agent.
    Can be customised in the constructor.
    """

    def __init__(
            self,
            inputs: List[int],
            outputs: int,
            feature_extractors: List[str],
            kernel_sizes: List[int] = None,
            kernel_strides: List[int] = None,
            recurrent=False,
            hidden_size=128
    ):
        super().__init__()
        self.base = CustomBase(inputs, feature_extractors,
                               kernel_sizes, kernel_strides, recurrent,
                               hidden_size)

        self.dist = Bernoulli(self.base.output_size, outputs)

    @property
    def is_recurrent(self):
        """
        Whether or not the policy contains a recurrent cell.
        """
        return self.base.is_recurrent

    @property
    def recurrent_hidden_state_size(self):
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

    def act(self, inputs, rnn_hxs, masks, deterministic=False):
        """
        Get a value, action, log probabilities for the action and the hidden
        states for the inputs given.
        """
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

    def evaluate_actions(self, inputs, rnn_hxs, masks, action):
        """
        Given a set of states and actions, evaluate taking those action in
        those states.
        """
        value, actor_features, rnn_hxs = self.base(inputs, rnn_hxs, masks)
        dist = self.dist(actor_features)

        action_log_probs = dist.log_probs(action)
        dist_entropy = dist.entropy().mean()

        return value, action_log_probs, dist_entropy, rnn_hxs


class CustomBase(NNBase):
    """
    Builds a set of feature extractors and allows for using them to process
    images.
    """

    def __init__(
            self,
            inputs: List[int],
            feature_extractors: List[str],
            kernel_sizes: List[int] = None,
            kernel_strides: List[int] = None,
            recurrent=False,
            hidden_size=128
    ):
        super().__init__(recurrent, hidden_size, hidden_size)

        def init_extractor(module):
            return init(module,
                        nn.init.orthogonal_,
                        lambda x: nn.init.constant_(x, 0),
                        nn.init.calculate_gain('relu'))

        # Feature extractors
        self.feature_extractors = nn.ModuleList()
        for i, extractor in enumerate(feature_extractors):
            if extractor == 'mlp':
                input_size = inputs[i]
                self.feature_extractors.append(
                    MLPExtractor(input_size, 32, init_extractor))
            if extractor == 'cnn':
                input_shape = inputs[i]
                self.feature_extractors.append(
                    CNNExtractor(input_shape, kernel_sizes, kernel_strides,
                                 init_extractor))

        # Feature size
        temp_inputs = [torch.zeros(x).unsqueeze(0) for x in inputs]
        temp_features = [self.feature_extractors[i](x)
                         for i, x in enumerate(temp_inputs)]
        self.feature_size = torch.cat(temp_features, dim=1).shape[1]

        self.linear = init_extractor(nn.Linear(self.feature_size, hidden_size))

        def init_critic(module):
            return init(module,
                        nn.init.orthogonal_,
                        lambda x: nn.init.constant_(x, 0))

        self.critic_linear = init_critic(nn.Linear(hidden_size, 1))

        self.train()

    def forward(self, x, rnn_hxs=None, masks=None) -> Tuple[torch.Tensor,
                                                            torch.Tensor,
                                                            torch.Tensor]:
        features = [extractor(x[i]) for i, extractor in enumerate(
            self.feature_extractors)]
        x = torch.cat(features, dim=-1)
        x = x.view(-1, self.feature_size)
        x = self.linear(x)
        x = F.relu(x)

        if self.is_recurrent:
            x, rnn_hxs = self._forward_gru(x, rnn_hxs, masks)

        return self.critic_linear(x), x, rnn_hxs


class MLPExtractor(nn.Module):
    """
    Feature extractor using a multi-layer perceptron to determine features.
    """

    def __init__(self, input_size, output_size, init_function):
        super().__init__()
        self.layer_1 = init_function(nn.Linear(input_size, 128))
        self.layer_2 = init_function(nn.Linear(128, output_size))

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = self.layer_1(x)
        x = F.relu(x)
        x = self.layer_2(x)
        x = F.relu(x)
        return x


class CNNExtractor(nn.Module):
    """
    Feature extractor using a convolutional neural network to determine
    features.
    """

    def __init__(
            self,
            input_shape,
            kernel_sizes,
            kernel_strides,
            init_function):
        super().__init__()
        self.layer_1 = init_function(nn.Conv2d(
            input_shape[0], 32, kernel_sizes[0], kernel_strides[0]))
        self.layer_2 = init_function(nn.Conv2d(
            32, 64, kernel_sizes[1], kernel_strides[1]))
        self.layer_3 = init_function(nn.Conv2d(
            64, 32, kernel_sizes[2], kernel_strides[2]))

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = self.layer_1(x)
        x = F.relu(x)
        x = self.layer_2(x)
        x = F.relu(x)
        x = self.layer_3(x)
        x = F.relu(x)
        return x.view(x.shape[0], -1)
