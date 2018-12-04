"""
Classes for building requests to send to the training server.
"""
from abc import ABC, abstractmethod
from typing import List
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
                "training": False,
                "contexts": self.contexts,
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


class GetActionsRequest(Request):
    """
    Builds the JSON for a request to get an action.
    """

    def __init__(self,
                 observation: np.ndarray,
                 session_id: int):
        self.session_id = session_id
        self.observation = observation

    def to_json(self) -> str:
        request = {
            "jsonrpc": "2.0",
            "method": "get_actions",
            "param": {
                "inputs": self.observation.tolist(),
                "session_id": self.session_id
            },
            "id": 0
        }
        return rapidjson.dumps(request)


class GiveRewardsRequest(Request):
    """
    Builds the JSON for a request to give a reward to an agent.
    """

    def __init__(self,
                 reward: List[float],
                 done: List[bool],
                 session_id: int):
        self.reward = reward
        self.done = done
        self.session_id = session_id

    def to_json(self) -> str:
        request = {
            "jsonrpc": "2.0",
            "method": "give_rewards",
            "param": {
                "reward": self.reward,
                "session_id": self.session_id,
                "done": self.done
            },
            "id": 0
        }
        return rapidjson.dumps(request)


class EndSessionRequest(Request):
    """
    Builds the JSON for a request to end a session.
    """

    def __init__(self, session_id: int):
        self.session_id = session_id

    def to_json(self) -> str:
        request = {
            "jsonrpc": "2.0",
            "method": "end_session",
            "param": {
                "session_id": self.session_id
            },
            "id": 0
        }
        return rapidjson.dumps(request)


class CloseConnectionRequest(Request):
    """
    Builds the JSON for a request to close the connection.
    """

    def to_json(self) -> str:
        request = {
            "jsonrpc": "2.0",
            "method": "close_connection",
            "param": [],
            "id": 0
        }
        return rapidjson.dumps(request)
