"""
Inference session
"""
from typing import List, Tuple
import torch

from .model import Model
from .train import ModelSpecification


class InferenceSession:
    """
    Manages an inference session for an agent.
    """

    def __init__(
            self,
            model: ModelSpecification,
            model_path: str,
            contexts: int):
        self.contexts = contexts

        self.model = Model(model.inputs, model.outputs,
                           model.feature_extractors, model.kernel_sizes,
                           model.kernel_strides)
        self.model.load_state_dict(torch.load(model_path))

    def get_action(
            self,
            inputs: List[torch.Tensor],
            _: None = None) -> Tuple[List[int], torch.Tensor]:
        """
        Given an observation, get an action and the value of the observation
        from one of the models being trained.
        """
        with torch.no_grad():
            value, probs, _ = self.model.act(inputs)

        actions = [x.multinomial(num_samples=1) for x in probs]

        return actions, value
