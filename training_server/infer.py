"""
Inference session
"""
from typing import List, Tuple
import torch

from .model import CustomPolicy
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

        self.model = CustomPolicy(model.inputs, model.outputs, model.recurrent)
        self.model.load_state_dict(torch.load(model_path))

    def get_actions(
            self,
            inputs: List[torch.Tensor],
            _: None = None) -> Tuple[List[int], torch.Tensor]:
        """
        Given an observation, get an action and the value of the observation
        from one of the models being trained.
        """
        inputs = torch.Tensor(inputs)
        with torch.no_grad():
            value, actions, _, _ = self.model.act(inputs, None, None)

        return actions, value
