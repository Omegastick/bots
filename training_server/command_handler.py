"""
Command handler
"""

from typing import NamedTuple, Any
import rapidjson

from .session_manager import SessionManager
from .model import ModelSpecification
from .train import HyperParams

BAD_REQUEST = ('{"jsonrpc":"2.0",'
               '"error":{"code":-32600,"message":"Invalid Request"},'
               '"id":null}')

PARSE_ERROR = ('{"jsonrpc":"2.0",'
               '"error":{"code":-32700,"message":"Parse error"},'
               '"id":null}')


class Command(NamedTuple):
    """
    A command recieved from the client.
    """
    action: str
    params: dict
    id: int


class Response(NamedTuple):
    """
    A response from the server to be sent to the client.
    """
    result: Any
    id: int


class BadRequestError(Exception):
    """
    Exception to throw when there is a mistake in the command.
    """
    pass


class BadJsonError(Exception):
    """
    Exception for when there is malformed JSON in the request.
    """
    pass


class CommandHandler:
    """
    Tool for handling JSON-RPC commands for the agent training server.
    """

    def __init__(self, session_manager: SessionManager):
        self.session_manager = session_manager

    def handle_command(self, command_json: str) -> str:
        """
        Receives a command in JSON-RPC format, handles it, then sends a
        response.
        """
        command: Command
        try:
            command = self.parse_json(command_json)
        except BadRequestError:
            return BAD_REQUEST
        except BadJsonError:
            return PARSE_ERROR

        params = command.params
        try:
            if command.action == "begin_session":
                model = ModelSpecification(
                    inputs=params["model"]["inputs"],
                    outputs=params["model"]["outputs"],
                    feature_extractors=params["model"]["feature_extractors"]
                )
                if params["training"]:
                    hyperparams = HyperParams(**params["hyperparams"])
                    self.session_manager.start_training_session(
                        params["session_id"], model, hyperparams,
                        params["contexts"], params["auto_train"]
                    )
                    response = Response(result="OK", id=command.id)
                    return self.create_response_json(response)
                else:
                    self.session_manager.start_inference_session(
                        params["session_id"], model, params["model_path"],
                        params["contexts"]
                    )
                    response = Response(result="OK", id=command.id)
                    return self.create_response_json(response)
            elif command.action == "get_action":
                actions, value = self.session_manager.get_action(
                    params["session_id"], params["inputs"], params["context"])
                value = value.item()
                actions = [action.item() for action in actions]
                response = Response(
                    id=command.id,
                    result={"actions": actions, "value": value}
                )
                return self.create_response_json(response)
            elif command.action == "give_reward":
                self.session_manager.give_reward(params["session_id"],
                                                 params["reward"],
                                                 params["context"])
                response = Response(result="OK", id=command.id)
                return self.create_response_json(response)
            elif command.action == "end_session":
                self.session_manager.end_session(params["session_id"])
                response = Response(result="OK", id=command.id)
                return self.create_response_json(response)

        except KeyError:
            return BAD_REQUEST

    def parse_json(self, json: str) -> Command:
        """
        Converts JSON into a Command.
        """
        try:
            obj = rapidjson.loads(json)
        except ValueError as exception:
            print(json)
            print(exception)
            raise BadJsonError

        if (
                "jsonrpc" not in obj
                or obj["jsonrpc"] != "2.0"
                or "param" not in obj
                or (not isinstance(obj["param"], list)
                    and not isinstance(obj["param"], dict))
                or "id" not in obj
                or not isinstance(obj["id"], int)
                or "method" not in obj
                or not isinstance(obj["method"], str)
        ):
            raise BadRequestError

        command = Command(
            id=obj["id"],
            action=obj["method"],
            params=obj["param"]
        )

        return command

    def create_response_json(self, response: Response) -> str:
        """
        Converts a Response to a JSON-RPC response.
        """
        obj = {
            "jsonrpc": "2.0",
            "result": response.result,
            "id": response.id
        }
        json = rapidjson.dumps(obj)

        return json
