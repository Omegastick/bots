"""
Command handler
"""

from typing import NamedTuple, Any
import rapidjson

from .session_manager import SessionManager

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

    def handle_command(self, command: str) -> str:
        """
        Receives a command in JSON-RPC format, handles it, then sends a
        response.
        """
        try:
            command = self.parse_json(command)
        except BadRequestError:
            return BAD_REQUEST
        except BadJsonError:
            return PARSE_ERROR

    def parse_json(self, json: str) -> Command:
        """
        Converts JSON into a Command.
        """
        try:
            obj = rapidjson.loads(json)
        except ValueError:
            raise BadJsonError

        if (
                "jsonrpc" not in obj
                or obj["jsonrpc"] != "2.0"
                or "param" not in obj
                or (not isinstance(obj["param"], list)
                    and not isinstance(obj["param"], dict))
                or "id" not in obj
                or not isinstance(obj["id"], int)
        ):
            raise BadRequestError
        return "Hi"

    def create_response_json(self, response: Response) -> str:
        """
        Converts a Response to a JSON-RPC response.
        """
        raise NotImplementedError
