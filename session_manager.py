"""
Interface between the frontend and sessions.
"""
from typing import Dict, Union

from model import ModelSpecification
from train import TrainingSession, HyperParams
from infer import InferenceSession


class SessionManager:
    def __init__(self):
        self.sessions: Dict[int, Union[TrainingSession, InferenceSession]] = {}

    def start_training_session(
        self,
        session_id: int,
        model: ModelSpecification,
        hyperparams: HyperParams,
        contexts: int,
        auto_train: bool
    ) -> None:
        assert session_id not in self.sessions, \
            f"Session with that ID already exists"
        session = TrainingSession(model, hyperparams, contexts, auto_train)
        self.sessions[session_id] = session
