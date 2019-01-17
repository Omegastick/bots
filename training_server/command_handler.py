"""
Command handler
"""

from typing import NamedTuple, Any, Optional
import logging
import msgpack

from .session_manager import SessionManager
from .model import ModelSpecification
from .train import HyperParams

BAD_REQUEST = msgpack.packb(
    {
        "api": "v1alpha1",
        "error": {"code": -32600, "message": "Invalid request"},
        "id": None
    })

PARSE_ERROR = msgpack.packb(
    {
        "api": "v1alpha1",
        "error": {"code": -32700, "message": "Parse error"},
        "id": None
    })


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


class BadMessageError(Exception):
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

    def handle_command(self, command_msg: bytes) -> bytes:
        """
        Receives a command in JSON-RPC format, handles it, then sends a
        response.
        """
        command: Command
        try:
            command = self.parse(command_msg)
        except BadRequestError:
            logging.debug(command_msg)
            return BAD_REQUEST
        except BadMessageError:
            logging.debug(command_msg)
            return PARSE_ERROR

        try:
            if command.action == "begin_session":
                return self.begin_session(command)
            if command.action == "get_actions":
                return self.get_actions(command)
            if command.action == "give_rewards":
                return self.give_rewards(command)
            if command.action == "end_session":
                return self.end_session(command)
            if command.action == "save_model":
                return self.save_model(command)
            if command.action == "close_connection":
                return self.close_connection()

            # Method not found
            return msgpack.packb(
                {
                    "api": "v1alpha1",
                    "error": {"code": -32601, "message": "Method not found"},
                    "id": command.id
                })
        except KeyError as exception:
            logging.debug(command_msg)
            logging.error(exception)
            import pdb
            pdb.post_mortem()
            return BAD_REQUEST

    def parse(self, message: str) -> Command:
        """
        Converts message into a Command.
        """
        try:
            obj = msgpack.unpackb(message, raw=False)
        except ValueError as exception:
            logging.warning(message)
            logging.warning(exception)
            raise BadMessageError

        if (
                "api" not in obj
                or obj["api"] != "v1alpha1"
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

    def create_response(self, response: Response) -> str:
        """
        Converts a Response to a valid API response.
        """
        obj = {
            "api": "v1alpha1",
            "result": response.result,
            "id": response.id
        }
        message = msgpack.packb(obj)

        return message

    def begin_session(self, command: Command) -> str:
        """
        Begins a new session.
        """
        params = command.params
        logging.info("Beginning session: %i", params["session_id"])
        model = ModelSpecification(
            inputs=params["model"]["inputs"],
            outputs=params["model"]["outputs"],
            recurrent=params["model"].get("recurrent"),
            normalize_observations=params["model"].get(
                "normalize_observations")
        )
        if params["training"]:
            hyperparams = HyperParams(**params["hyperparams"])
            self.session_manager.start_training_session(
                params["session_id"], model, hyperparams, params["contexts"])
            response = Response(result="OK", id=command.id)
            return self.create_response(response)

        self.session_manager.start_inference_session(
            params["session_id"], model, params["model_path"],
            params["contexts"])
        response = Response(result="OK", id=command.id)
        return self.create_response(response)

    def get_actions(self, command: Command) -> str:
        """
        Gets an action from a session.
        """
        params = command.params
        actions, value = self.session_manager.get_actions(
            params["session_id"], params["inputs"])
        values = value.view(-1).tolist()
        actions = actions.byte().tolist()
        response = Response(
            id=command.id,
            result={"actions": actions, "values": values}
            # result={"actions": "Hello", "values": "I am Isaac"}
        )
        return self.create_response(response)

    def give_rewards(self, command: Command) -> str:
        """
        Gives a reward to an agent in a training session.
        """
        params = command.params
        self.session_manager.give_rewards(params["session_id"],
                                          params["rewards"],
                                          params["dones"])
        response = Response(result="OK", id=command.id)
        return self.create_response(response)

    def end_session(self, command: Command) -> str:
        """
        Ends a session.
        """
        params = command.params
        logging.info("Ending session: %i", params["session_id"])
        self.session_manager.end_session(params["session_id"])
        response = Response(result="OK", id=command.id)
        return self.create_response(response)

    def save_model(self, command: Command) -> str:
        """
        Saves a model.
        """
        params = command.params
        self.session_manager.save_model(params["session_id"], params["path"])
        response = Response(result="OK", id=command.id)
        return self.create_response(response)

    def close_connection(self) -> str:
        """
        Begins waiting for a new connection.
        """
        return "New connection"
