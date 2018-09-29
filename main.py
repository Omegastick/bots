"""
Main script
"""
import argparse

from training_server.session_manager import SessionManager
from training_server.server import Server


def main():
    """
    Hosts a ZeroMQ server and listens on it for commands to send to a
    SessionManager.
    """
    parser = argparse.ArgumentParser(description="Server for training agents.")
    parser.add_argument('--port', type=int, default=10201,
                        help="Port on which to host the server.")
    args = parser.parse_args()

    session_manager = SessionManager()
    server = Server(args.port)
    while True:
        message = server.get_message()


if __name__ == '__main__':
    main()
