"""
Command handler
"""

from typing import NamedTuple, Any, Optional
import logging
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
    id: Optional[int]


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
            logging.debug(command_json)
            return BAD_REQUEST
        except BadJsonError:
            logging.debug(command_json)
            return PARSE_ERROR

        try:
            if command.action == "begin_session":
                return self.begin_session(command)
            if command.action == "get_action":
                return self.get_action(command)
            if command.action == "give_reward":
                return self.give_reward(command)
            if command.action == "end_session":
                return self.end_session(command)
            if command.action == "close_connection":
                return self.close_connection()

            # Method not found
            return ('{"jsonrpc":"2.0",'
                    '"error":{"code":-32601,"message":"Method not found"},'
                    f'"id":{command.id}}}')
        except KeyError as exception:
            logging.debug(command_json)
            logging.error(exception)
            return BAD_REQUEST

    def parse_json(self, json: str) -> Command:
        """
        Converts JSON into a Command.
        """
        try:
            obj = rapidjson.loads(json)
        except ValueError as exception:
            logging.warning(json)
            logging.warning(exception)
            raise BadJsonError

        if (
                "jsonrpc" not in obj
                or obj["jsonrpc"] != "2.0"
                or "param" not in obj
                or (not isinstance(obj["param"], list)
                    and not isinstance(obj["param"], dict))
                or "method" not in obj
                or not isinstance(obj["method"], str)
        ):
            raise BadRequestError

        command = Command(
            id=obj.get("id", None),
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

    def begin_session(self, command: Command) -> str:
        """
        Begins a new session.
        """
        params = command.params
        model = ModelSpecification(
            inputs=params["model"]["inputs"],
            outputs=params["model"]["outputs"],
            feature_extractors=params["model"]["feature_extractors"]
        )
        # Inputs don't match feature extractors.
        if len(model.inputs) != len(model.feature_extractors):
            import ipdb; ipdb.set_trace()
            return ('{"jsonrpc":"2.0",'
                    '"error":{"code":-1000,'
                    '"message":"Feature extractors don\'t match inputs"},'
                    '"id":0}')
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

    def get_action(self, command: Command) -> str:
        """
        Gets an action from a session.
        """
        params = command.params
        actions, value = self.session_manager.get_action(
            params["session_id"], params["inputs"], params["context"])
        value = value.item()
        actions = [action.item() for action in actions]
        response = Response(
            id=command.id,
            result={"actions": actions, "value": value}
        )
        return self.create_response_json(response)

    def give_reward(self, command: Command) -> str:
        """
        Gives a reward to an agent in a training session.
        """
        params = command.params
        self.session_manager.give_reward(params["session_id"],
                                         params["reward"],
                                         params["context"])
        response = Response(result="OK", id=command.id)
        return self.create_response_json(response)

    def end_session(self, command: Command) -> str:
        """
        Ends a session.
        """
        params = command.params
        self.session_manager.end_session(params["session_id"])
        response = Response(result="OK", id=command.id)
        return self.create_response_json(response)

    def close_connection(self) -> str:
        """
        Begins waiting for a new connection.
        """
        return "New connection"
