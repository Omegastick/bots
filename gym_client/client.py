"""
Client.
"""
import logging
from requests import Request
import zmq
import msgpack


class Client:
    """
    Class for connecting to and communicating with the training_server.
    """

    def __init__(self, url: str = 'tcp://127.0.0.1:10201'):
        context = zmq.Context()
        self.socket = context.socket(zmq.PAIR)
        self.socket.connect(url)
        self.socket.send_string("Establishing connection...")
        logging.info(self.socket.recv_string())

    def send_request(self, request: Request) -> dict:
        """
        Sends a request to the server and gets the response.
        Returns the received message object in dictionary form.
        """
        self.socket.send(request.to_msg())
        response_msg = self.socket.recv()
        response = msgpack.unpackb(response_msg, raw=False)
        return response
