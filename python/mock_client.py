"""
A Python version of the multiplayer client for planning purposes.
"""
from threading import Thread
import time
import msgpack

import zmq

from mock_server import PLAYERS_PER_GAME

NUMBER_OF_CLIENTS = 2


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

        player_number = msgpack.unpackb(self._dealer.recv())
        print("Game starting")

        finished = False
        while not finished:
            # Send action
            action = None
            if player_number == 0:
                action = 5
            else:
                time.sleep(1.1)
                action = self._state[0] - self._state[1]
            self._dealer.send(msgpack.packb([
                self._current_tick,
                action
            ]))

            # Receive all messages from server and pick the latest
            messages = []
            while not messages:
                while True:
                    try:
                        packed_message = self._dealer.recv(flags=zmq.NOBLOCK)
                        unpacked_message = msgpack.unpackb(packed_message)
                        messages.append(unpacked_message)
                    except zmq.ZMQError:
                        break
            messages.sort(key=lambda message: message[0], reverse=True)

            # Update client
            message = messages[0]
            self._current_tick = message[0]
            self._state = message[1]
            finished = message[2]
            if player_number == 1:
                print("\r{}".format(self._state), end='')
                if finished:
                    print()
                    print(message[3])


def worker():
    """
    Launch a client and run a game.
    """
    client = Client()
    client.run()


def main():
    """
    Connect to a server and run a game.
    """
    threads = []
    for _ in range(NUMBER_OF_CLIENTS):
        threads.append(Thread(target=worker))
        threads[-1].start()

    for thread in threads:
        thread.join()


if __name__ == '__main__':
    main()
