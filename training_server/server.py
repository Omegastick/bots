"""
Server
"""
import zmq


class Server:
    """
    Provides a ZeroMQ server for retrieving commands from the client.
    """

    def __init__(self, port: int):
        context = zmq.Context()
        self.socket = context.socket(zmq.PAIR)
        self.socket.bind(f"tcp://*:{port}")

    def get_message(self) -> bytes:
        """
        Gets a message from the client.
        Blocks until a message is received.
        """
        message = self.socket.recv()
        return message

    def send_message(self, message: object):
        """
        Sends a message to the client.
        """
        if isinstance(message, str):
            self.socket.send_string(message)
        else:
            self.socket.send(message)
