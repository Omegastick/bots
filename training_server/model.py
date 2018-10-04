"""
Agent models
"""
from typing import List, Tuple, NamedTuple
import numpy as np
import torch
import torch.nn.functional as F


class ModelSpecification(NamedTuple):
    """
    Named tuple for specifying a model to be created later.
    """
    inputs: List[int]
    outputs: List[int]
    feature_extractors: List[str]


def normalized_columns_initializer(
        weights: torch.Tensor,
        std: float = 1.):
    """
    Normalises each layer's weights when initialising.
    Based on: https://github.com/ikostrikov/pytorch-a3c/blob/master/model.py
    """
    out = torch.randn(weights.size())
    out *= std / torch.sqrt(out.pow(2).sum(1, keepdim=True))
    return out


def weights_init(module: torch.nn.Module) -> torch.Tensor:
    """
    Weight initialisation function magic.
    Based on: https://github.com/ikostrikov/pytorch-a3c/blob/master/model.py
    """
    classname = module.__class__.__name__
    if classname.find('Conv') != -1:
        weight_shape = list(module.weight.data.size())
        fan_in = np.prod(weight_shape[1:4])
        fan_out = np.prod(weight_shape[2:4]) * weight_shape[0]
        w_bound = np.sqrt(6. / (fan_in + fan_out))
        module.weight.data.uniform_(-w_bound, w_bound)
        module.bias.data.fill_(0)
    elif classname.find('Linear') != -1:
        weight_shape = list(module.weight.data.size())
        fan_in = weight_shape[1]
        fan_out = weight_shape[0]
        w_bound = np.sqrt(6. / (fan_in + fan_out))
        module.weight.data.uniform_(-w_bound, w_bound)
        module.bias.data.fill_(0)


class MLPExtractor(torch.nn.Module):
    """
    Feature extractor using a multi-layer perceptron to determine features.
    """

    def __init__(self, input_size, output_size):
        super().__init__()
        self.layer_1 = torch.nn.Linear(input_size, 128)
        self.layer_2 = torch.nn.Linear(128, 128)
        self.layer_3 = torch.nn.Linear(128, output_size)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = self.layer_1(x)
        x = F.relu(x)
        x = self.layer_2(x)
        x = F.relu(x)
        x = self.layer_3(x)
        x = F.relu(x)
        return x


class Model(torch.nn.Module):
    """
    Default neural network model used by agents.
    """

    def __init__(
            self,
            inputs: List[int],
            outputs: List[int],
            feature_extractors: List[str]):
        super().__init__()
        self.feature_extractors = torch.nn.ModuleList()
        for i, extractor in enumerate(feature_extractors):
            if extractor == 'mlp':
                input_size = inputs[i]
                self.feature_extractors.append(
                    MLPExtractor(input_size, 32))

        temp_inputs = [torch.zeros(x) for x in inputs]
        temp_features = [self.feature_extractors[i](x)
                         for i, x in enumerate(temp_inputs)]
        self.feature_size = torch.cat(temp_features).shape[0]

        self.critic = torch.nn.Linear(self.feature_size, 1)

        self.actors = torch.nn.ModuleList()
        for output in outputs:
            self.actors.append(torch.nn.Linear(self.feature_size, output))

        self.apply(weights_init)

        for actor in self.actors:
            actor.weight.data = normalized_columns_initializer(
                actor.weight.data, 0.01)
            actor.bias.data.fill_(0)
        self.critic.weight.data = normalized_columns_initializer(
            self.critic.weight.data, 1.0)
        self.critic.bias.data.fill_(0)

    def forward(
            self,
            x: List[torch.Tensor]) -> Tuple[torch.Tensor, List[torch.Tensor]]:
        features = [extractor(x[i]) for i, extractor in enumerate(
            self.feature_extractors)]
        x = torch.cat(features, dim=-1)
        x = F.relu(x)
        value = self.critic(x)
        raw_probs = [actor(x) for actor in self.actors]
        return value, raw_probs

    def get_value(
            self,
            x: List[torch.Tensor]) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Gets the predicted value for an observation.
        """
        features = [extractor(x[i]) for i, extractor in enumerate(
            self.feature_extractors)]
        x = torch.cat(features, dim=-1)
        x = F.relu(x)
        value = self.critic(x)
        return value

    def act(
            self,
            x: List[torch.Tensor]) -> Tuple[torch.Tensor, torch.Tensor,
                                            torch.Tensor]:
        """
        Get the predicted value, action probabilities and the log probabilities
        of an observation.
        """
        value, raw_probs = self(x)
        probs = [F.softmax(raw_prob, dim=0) for raw_prob in raw_probs]
        log_probs = [F.log_softmax(raw_prob, dim=0) for raw_prob in raw_probs]

        return value, probs, log_probs
