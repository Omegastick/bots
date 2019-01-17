#!/usr/bin/env python
"""
Main script
"""
# pylint: disable=wrong-import-position
import os
os.environ['OMP_NUM_THREADS'] = '1'
import argparse
import logging

from training_server.session_manager import SessionManager
from training_server.server import Server
from training_server.command_handler import CommandHandler

def main():
    """
    Hosts a ZeroMQ server and listens on it for commands to send to a
    SessionManager.
    """
    logging.basicConfig(level=logging.DEBUG,
                        format=('%(asctime)s %(funcName)s '
                                '[%(levelname)s]: %(message)s'),
                        datefmt='%Y%m%d %H:%M:%S')

    os.environ['OMP_NUM_THREADS'] = '1'

    parser = argparse.ArgumentParser(description="Server for training agents.")
    parser.add_argument('--port', type=int, default=10201,
                        help="Port on which to host the server.")
    args = parser.parse_args()

    session_manager = SessionManager()
    command_handler = CommandHandler(session_manager)

    server = Server(args.port)
    logging.info("Server started...")
    server.send_message("Connection established...")
    logging.debug(server.get_message().decode())

    while True:
        message = server.get_message()
        response = command_handler.handle_command(message)
        server.send_message(response)


if __name__ == '__main__':
    main()
