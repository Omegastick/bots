"""
A Python version of the multiplayer client for planning purposes.
"""
import random

import msgpack
import zmq

from mock_server import PLAYERS_PER_GAME


class Client:
    """
    Connects to a server to play games.
    """

    def __init__(self):
        context = zmq.Context.instance()
        self._dealer: zmq.Socket = context.socket(zmq.DEALER)
        self._dealer.connect("tcp://localhost:7654")

        self._current_tick = 0
        self._state = [0 for _ in range(PLAYERS_PER_GAME)]

    def run(self):
        """
        Connect to the server.
        """
        # Get ID
        self._dealer.send(b'ID')
        new_id = self._dealer.recv_string()
        self._dealer.setsockopt_string(zmq.IDENTITY, new_id)
        self._dealer.disconnect("tcp://localhost:7654")
        self._dealer.connect("tcp://localhost:7654")

        self._dealer.send(b'Game plz')
        print("Sent game request")

        self._dealer.recv()
        print("Game starting")

        action = random.randint(-5, 5)

        while True:
            self._dealer.send(msgpack.packb([self._current_tick, action]))
            packed_message = self._dealer.recv()
            message = msgpack.unpackb(packed_message)
            self._current_tick = message[0]
            self._state = message[1]
            print(self._state)


def main():
    """
    Connect to a server and run a game.
    """
    client = Client()
    client.run()


if __name__ == '__main__':
    main()
