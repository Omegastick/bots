"""
Classes for building requests to send to the training server.
"""
from abc import ABC, abstractmethod
import numpy as np
import rapidjson
from training_server.model import ModelSpecification
from training_server.train import HyperParams


class Request(ABC):
    """
    Base class for requests.
    """
    @abstractmethod
    def to_json(self) -> str:
        """
        Creates the JSON for the request.
        """
        pass


class BeginTrainingSessionRequest(Request):
    """
    Builds the JSON for a request to begin a training session.
    """

    def __init__(self,
                 model: ModelSpecification,
                 hyperparams: HyperParams,
                 contexts: int,
                 session_id: int):
        self.model = model
        self.hyperparams = hyperparams
        self.contexts = contexts
        self.session_id = session_id

    def to_json(self) -> str:
        request = {
            "jsonrpc": "2.0",
            "method": "begin_session",
            "param": {
                "model": self.model._asdict(),
                "hyperparams": self.hyperparams._asdict(),
                "session_id": self.session_id,
                "training": True,
                "contexts": self.contexts,
                "auto_train": True
            },
            "id": 0
        }
        return rapidjson.dumps(request)


class BeginInferenceSessionRequest(Request):
    """
    Builds the JSON for a request to begin an inference session.
    """

    def __init__(self,
                 model: ModelSpecification,
                 contexts: int,
                 session_id: int,
                 model_path: str):
        self.model = model
        self.contexts = contexts
        self.session_id = session_id
        self.model_path = model_path

    def to_json(self) -> str:
        request = {
            "jsonrpc": "2.0",
            "method": "begin_session",
            "param": {
                "model": self.model._asdict(),
                "session_id": self.session_id,
                "training": True,
                "contexts": self.contexts,
                "auto_train": True,
                "model_path": self.model_path
            },
            "id": 0
        }
        return rapidjson.dumps(request)


class SaveModelRequest(Request):
    """
    Builds the JSON for a request to save a model.
    """

    def __init__(self,
                 path: str,
                 session_id: int):
        self.session_id = session_id
        self.path = path

    def to_json(self) -> str:
        request = {
            "jsonrpc": "2.0",
            "method": "save_model",
            "param": {
                "path": self.path,
                "session_id": self.session_id
            },
            "id": 0
        }
        return rapidjson.dumps(request)


class GetActionRequest(Request):
    """
    Builds the JSON for a request to get an action.
    """

    def __init__(self,
                 observation: np.ndarray,
                 context: int,
                 session_id: int):
        self.session_id = session_id
        self.observation = observation
        self.context = context

    def to_json(self) -> str:
        request = {
            "jsonrpc": "2.0",
            "method": "get_action",
            "param": {
                "inputs": self.observation,
                "context": self.context,
                "session_id": self.session_id
            },
            "id": 0
        }
        return rapidjson.dumps(request)


class GiveRewardRequest(Request):
    """
    Builds the JSON for a request to give a reward to an agent.
    """

    def __init__(self,
                 reward: float,
                 context: int,
                 session_id: int):
        self.session_id = session_id
        self.reward = reward
        self.context = context

    def to_json(self) -> str:
        request = {
            "jsonrpc": "2.0",
            "method": "give_reward",
            "param": {
                "reward": self.reward,
                "context": self.context,
                "session_id": self.session_id
            },
            "id": 0
        }
        return rapidjson.dumps(request)
