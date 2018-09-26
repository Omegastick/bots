import torch
import torch.nn.functional as F


class MLPExtractor(torch.nn.Module):
    def __init__(self, input_size, output_size):
        layer_1 = torch.nn.Linear(input_size, 128)
        layer_2 = torch.nn.Linear(128, 128)
        layer_3 = torch.nn.Linear(128, output_size)

    def forward(self, x):
        x = self.layer_1(x)
        x = F.relu(x)
        x = self.layer_2(x)
        x = F.relu(x)
        x = self.layer_3(x)
        x = F.relu(x)
        return x


class Model(torch.nn.Module):
    def __init__(
        self,
        inputs,
        outputs,
        feature_extractors,
        hidden_layers
    ):
        self.feature_extractors = torch.nn.ModuleList
        for i, extractor in enumerate(feature_extractors):
            if extractor == 'mlp':
                input_size = inputs[i].shape(0)
                self.feature_extractors.append(
                    MLPExtractor(input_size, 32))

        temp_inputs = [torch.zeros(*x) for x in inputs]
        temp_features = [self.feature_extractors[i](x)
                         for i, x in enumerate(temp_inputs)]
        self.feature_size = torch.cat(temp_features).shape(0)

        self.hidden_layers = torch.nn.Sequential(
            torch.nn.Linear(self.feature_size, 128),
            *[torch.nn.Linear(128, 128) for _ in range(hidden_layers)]
        )

        self.
