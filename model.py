"""
Agent models.
"""
import torch
import torch.nn.functional as F


class MLPExtractor(torch.nn.Module):
    """
    Feature extractor using a multi-layer perceptron to determine features.
    """

    def __init__(self, input_size, output_size):
        super().__init__()
        self.layer_1 = torch.nn.Linear(input_size, 128)
        self.layer_2 = torch.nn.Linear(128, 128)
        self.layer_3 = torch.nn.Linear(128, output_size)

    def forward(self, x):
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
        inputs,
        outputs,
        feature_extractors
    ):
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

    def forward(self, x):
        features = [extractor(x[i]) for i, extractor in enumerate(
            self.feature_extractors)]
        x = torch.cat(features)
        x = F.relu(x)
        value = self.critic(x)
        raw_probs = [actor(x) for actor in self.actors]
        return value, raw_probs

    def act(self, x):
        """
        Get the predicted value, action probabilities and the log probabilities
        of an observation.
        """
        value, raw_probs = self(x)
        probs = [F.softmax(raw_prob, dim=0) for raw_prob in raw_probs]
        log_probs = [F.log_softmax(raw_prob, dim=0) for raw_prob in raw_probs]

        return value, probs, log_probs
