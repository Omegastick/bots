"""
Tests for server.py
"""
#pylint: disable=W0621
import pytest
import zmq

from training_server.server import Server
from .util import setup # pylint: disable=W0611


@pytest.fixture()
def server():
    """
    Functioning ZMQ server.
    """
    return Server(10205)


@pytest.fixture()
def client():
    """
    ZMQ client socket.
    """
    context = zmq.Context()
    socket = context.socket(zmq.PAIR)
    socket.connect("tcp://127.0.0.1:10205")
    return socket


def test_send_message_sends_message(
        server: Server,
        client: zmq.Socket):
    """
    When sending a message, the client should receive the message sent.
    """
    expected = "Test"
    server.send_message(expected)
    message = client.recv().decode()
    assert message == expected


def test_get_message_gets_message(
        server: Server,
        client: zmq.Socket):
    """
    When receiving a message, the message should be the same as the one sent.
    """
    expected = "Test"
    client.send_string(expected)
    message = server.get_message()
    assert message == expected
