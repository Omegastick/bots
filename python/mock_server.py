"""
A Python version of the server architecture for planning purposes.
"""
import random
import string
import time
from typing import Dict
from queue import Queue

import msgpack
import zmq

GOAL = 25
ID_LENGTH = 8
PLAYERS_PER_GAME = 2
TICK_LENGTH = 1


def generate_id() -> bytearray:
    """
    Generates a random 8 letter ID for clients.
    """
    return ''.join(random.choices(string.ascii_letters, k=8)).encode('UTF-8')


class Game:
    """
    Very simple 'game' that just adds numbers.
    """

    def __init__(self, number_of_players, goal):
        self._last_tick_time = time.time()
        self._goal = goal

        self.actions = {0: [0 for _ in range(number_of_players)]}
        self.action_received = {0: [False for _ in range(number_of_players)]}
        self.current_tick = 0
        self.players = []
        self.state = [0 for _ in range(number_of_players)]

    def tick(self):
        """
        Tick the game once.
        Returns True if the game is finished, False if not.
        """
        self._last_tick_time = time.time()

        if self.current_tick not in self.actions:
            self.actions[self.current_tick] = [0 for _ in
                                               range(len(self.state))]
            self.action_received[self.current_tick] = [False for _ in
                                                       range(len(self.state))]

        for i, _ in enumerate(self.state):
            # If we haven't received an action for this step, use the latest
            # received action
            if not self.action_received[self.current_tick][i]:
                for step in sorted(self.action_received, reverse=True):
                    if self.action_received[step][i]:
                        self.actions[self.current_tick][i] = \
                            self.actions[step][i]
                        break
            self.state[i] += self.actions[self.current_tick][i]

        self.current_tick += 1

        victors = [state >= self._goal for state in self.state]
        if not any(victors):
            return None
        return victors.index(True)

    def ready_to_tick(self, current_time: float):
        """
        Check if enough time has passed since the last tick to do another one.
        """
        return current_time >= self._last_tick_time + 1


class Server:
    """
    Hosts multiple games to which clients can connect.
    """

    def __init__(self):
        context = zmq.Context.instance()

        self._games: Dict[Game, bytearray] = {}
        self._ingame_players: Dict[bytearray, bytearray] = {}
        self._waiting_players = Queue()

        self._router: zmq.Socket = context.socket(zmq.ROUTER)
        self._router.bind("tcp://*:7654")

    def run(self):
        """
        Begin the main server loop.
        """
        while True:
            # Handle all messages from the router
            try:
                while True:
                    address, message = self._router.recv_multipart(
                        flags=zmq.NOBLOCK)
                    self._handle_message(address, message)
            except zmq.ZMQError:
                pass

            # Create any necessary new games
            self._create_games()

            # Tick games
            self._tick_games()

    def _create_games(self):
        """
        Assigns waiting players to new games.
        """
        while self._waiting_players.qsize() >= PLAYERS_PER_GAME:
            game_id = generate_id()
            self._games[game_id] = Game(PLAYERS_PER_GAME, GOAL)
            for _ in range(PLAYERS_PER_GAME):
                player_id = self._waiting_players.get()
                self._ingame_players[player_id] = game_id
                self._games[game_id].players.append(player_id)
                self._router.send_multipart([
                    player_id,
                    msgpack.packb(len(self._games[game_id].players) - 1)
                ])

    def _handle_message(self, address: bytearray, message: bytearray):
        """
        Given a message from the router, handles it appropriately.
        """
        if message == b'ID':
            self._router.send_multipart([
                address,
                generate_id()
            ])

        elif message == b'Game plz':
            print(f"Client {address} waiting for game")
            self._waiting_players.put(address)

        else:
            game = self._games[self._ingame_players[address]]
            index = game.players.index(address)
            decoded_message = msgpack.unpackb(message)
            if decoded_message[0] not in game.actions:
                game.actions[decoded_message[0]] = [
                    0 for _ in range(len(game.state))]
                game.action_received[decoded_message[0]] = [
                    False for _ in range(len(game.state))]
            game.actions[decoded_message[0]][index] = decoded_message[1]
            game.action_received[decoded_message[0]][index] = True

        return

    def _tick_games(self):
        """
        Run all games for one tick.
        """
        current_time = time.time()
        for game_id in self._games:
            game = self._games[game_id]
            if (game.ready_to_tick(current_time)
                    and all(game.action_received[0])):
                victor = game.tick()
                done = False
                if victor is not None:
                    done = True
                for player_id in self._games[game_id].players:
                    self._router.send_multipart([
                        player_id,
                        msgpack.packb([
                            game.current_tick,
                            game.state,
                            done,
                            victor
                        ])
                    ])


def main():
    """
    Run the server simulation.
    """
    server = Server()
    server.run()


if __name__ == '__main__':
    main()
