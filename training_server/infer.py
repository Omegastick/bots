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

        self.hidden_states = torch.zeros(contexts, 128)

        self.model = CustomPolicy(model.inputs, model.outputs, model.recurrent)
        self.model.load_state_dict(torch.load(model_path))

    def get_actions(
            self,
            obs: List[List[float]]
    ) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Given an observation, get an action and the value of the observation
        from the models being trained.
        """
        obs = torch.Tensor(obs)

        with torch.no_grad():
            values, actions, _, self.hidden_states = self.model.act(
                obs,
                self.hidden_states,
                torch.FloatTensor([[0]] * self.contexts)
            )

        return actions, values
