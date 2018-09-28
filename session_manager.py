"""
Interface between the frontend and sessions.
"""
from typing import Dict, Union, List
import numpy as np
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
            inputs: List[np.ndarray],
            context: int):
        """
        Get an action from a session.
        """
        for i, _input in enumerate(inputs):
            inputs[i] = torch.from_numpy(_input).float()
        return self.sessions[session_id].get_action(inputs, context)

    def give_reward(self, session_id: int, reward: float, context: int):
        """
        Gives a reward to an agent in a training session.
        """
        assert isinstance(self.sessions[session_id], TrainingSession), \
            "Can only give rewards to training sessions"
        self.sessions[session_id].give_reward(reward, context)
