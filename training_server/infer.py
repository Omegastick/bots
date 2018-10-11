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
                           model.feature_extractors)
        self.model.load_state_dict(torch.load(model_path))

    def get_action(
            self,
            inputs: List[torch.Tensor],
            _: None = None,
            hidden_state: torch.Tensor = None) -> Tuple[List[int], torch.Tensor]:
        """
        Given an observation, get an action and the value of the observation
        from one of the models being trained.
        """
        if hidden_state is None:
            hidden_state = torch.zeros(1, 128)
        value, probs, _, hidden_state = self.model.act(inputs, hidden_state)

        actions = [x.multinomial(num_samples=1) for x in probs]

        return actions, value, hidden_state
