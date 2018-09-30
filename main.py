"""
Main script
"""
import argparse
import time

from training_server.session_manager import SessionManager
from training_server.server import Server
from training_server.command_handler import CommandHandler


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
    command_handler = CommandHandler(session_manager)

    server = Server(args.port)
    print("Server started...")
    server.send_message("Connection established...")
    server.get_message()

    cycle_counter = 0
    last_display_time = time.time()
    while True:
        message = server.get_message()
        response = command_handler.handle_command(message)
        server.send_message(response)
        cycle_counter += 1
        if time.time() - last_display_time > 1:
            print("FPS: "
                  f"{cycle_counter / (time.time() - last_display_time):.2f}")
            last_display_time = time.time()
            cycle_counter = 0


if __name__ == '__main__':
    main()
