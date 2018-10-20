"""
Session manager
"""
from typing import Dict, Union, List
import torch

from .model import ModelSpecification
from .train import TrainingSession, HyperParams
from .infer import InferenceSession


class SessionManager:
    """
    Manages training and inference sessions.
    """

    def __init__(self):
        self.sessions: Dict[int, Union[TrainingSession, InferenceSession]] = {}

    def start_training_session(
            self,
            session_id: int,
            model: ModelSpecification,
            hyperparams: HyperParams,
            contexts: int,
            auto_train: bool):
        """
        Begins a training session.
        """
        assert session_id not in self.sessions, \
            "Session with that ID already exists"
        session = TrainingSession(model, hyperparams, contexts, auto_train)
        self.sessions[session_id] = session

    def start_inference_session(
            self,
            session_id: int,
            model: ModelSpecification,
            model_path: str,
            contexts: int):
        """
        Begins an inference session.
        """
        assert session_id not in self.sessions, \
            "Session with that ID already exists"
        session = InferenceSession(model, model_path, contexts)
        self.sessions[session_id] = session

    def get_action(
            self,
            session_id: int,
            inputs: List[list],
            context: int = None):
        """
        Get an action from a session.
        """
        for i, _input in enumerate(inputs):
            inputs[i] = torch.Tensor(_input).float().unsqueeze(0)
        return self.sessions[session_id].get_action(inputs, context)

    def give_reward(self, session_id: int, reward: float, context: int):
        """
        Gives a reward to an agent in a training session.
        """
        assert isinstance(self.sessions[session_id], TrainingSession), \
            "Can only give rewards to training sessions."
        self.sessions[session_id].give_reward(reward, context)

    def save_model(self, session_id: int, path: str):
        """
        Saves a model to disk.
        """
        assert isinstance(self.sessions[session_id], TrainingSession), \
            "Can only save models from training sessions."
        self.sessions[session_id].save_model(path)

    def end_session(self, session_id):
        """
        Ends a session.
        """
        assert session_id in self.sessions, \
            "Session doesn't exist."
        self.sessions.pop(session_id)
