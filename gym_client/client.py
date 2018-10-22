"""
Client.
"""
import logging
from requests import Request
import zmq
import rapidjson


class Client:
    """
    Class for connecting to and communicating with the training_server.
    """

    def __init__(self, url: str = 'tcp://127.0.0.1:10201'):
        context = zmq.Context()
        self.socket = context.socket(zmq.PAIR)
        self.socket.connect(url)
        logging.info(self.socket.recv_string())

    def send_request(self, request: Request) -> dict:
        """
        Sends a request to the server and gets the response.
        Returns the received JSON object in dictionary form.
        """
        self.socket.send_string(request.to_json())
        response_json = self.socket.recv_string()
        response = rapidjson.loads(response_json)
        return response
