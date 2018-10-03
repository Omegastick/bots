"""
Tests for command_handler.py
"""
#pylint: disable=W0621
import os
import pytest
from pytest_mock import mocker, MockFixture  # pylint: disable=W0611

from training_server.command_handler import CommandHandler, Command, Response
from training_server.session_manager import SessionManager
from training_server.model import ModelSpecification
from training_server.train import HyperParams
from .util import setup # pylint: disable=W0611


@pytest.fixture()
def session_manager():
    """
    A functioning SessionManager
    """
    return SessionManager()


@pytest.fixture()
def command_handler(session_manager):
    """
    A functioning CommandHandler
    """
    return CommandHandler(session_manager)


def test_parse_json_parses_basic_command(command_handler: CommandHandler):
    """
    When given a basic JSON-RPC command, the handler should create the right
    Command.
    """
    json = """
    {
        "jsonrpc": "2.0",
        "method": "test",
        "param": {
            "test1": "testa",
            "test2": "testb"
        },
        "id": 0
    }
    """
    command = command_handler.parse_json(json)

    expected = Command(
        action="test",
        params={"test1": "testa", "test2": "testb"},
        id=0
    )
    assert command.action == expected.action
    assert command.params == expected.params
    assert command.id == expected.id


def test_create_response_json_creates_basic_response(
        command_handler: CommandHandler):
    """
    When given a basic response, create_response_json should create the correct
    json.
    """
    response = Response(result=0.3, id=0)
    response_json = command_handler.create_response_json(response)
    expected = '{"jsonrpc":"2.0","result":0.3,"id":0}'
    assert response_json == expected


def test_begin_training_session_begins_training_session(
        command_handler: CommandHandler,
        session_manager: SessionManager,
        mocker: MockFixture):
    """
    When recieving a command telling the handler to being a training session,
    it should call the session manager and tell it to begin a training session.
    """
    mocker.spy(session_manager, 'start_training_session')
    request = """
    {
	    "jsonrpc": "2.0",
	    "method": "begin_session",
	    "param": {
		    "model": {
			    "inputs": [2, 4],
			    "outputs": [2, 1],
			    "feature_extractors": ["mlp", "mlp"]
			},
			"hyperparams": {
			    "learning_rate": 0.001,
			    "gae": 0.9,
			    "batch_size": 5
			},
            "session_id": 0,
			"training": true,
			"contexts": 1,
            "auto_train": true
		},
	    "id": 0
	}
    """
    command_handler.handle_command(request)

    assert session_manager.start_training_session.call_count == 1


def test_begin_training_session_returns_correct_json(
        command_handler: CommandHandler):
    """
    When recieving a command telling the handler to being a training session,
    it should call the session manager and tell it to begin a training session.
    """
    request = """
    {
	    "jsonrpc": "2.0",
	    "method": "begin_session",
	    "param": {
		    "model": {
			    "inputs": [2, 4],
			    "outputs": [2, 1],
			    "feature_extractors": ["mlp", "mlp"]
			},
			"hyperparams": {
			    "learning_rate": 0.001,
			    "gae": 0.9,
			    "batch_size": 5
			},
            "session_id": 0,
			"training": true,
			"contexts": 1,
            "auto_train": true
		},
	    "id": 0
	}
    """
    result = command_handler.handle_command(request)

    assert result == ('{"jsonrpc":"2.0",'
                      '"result":"OK",'
                      '"id":0}')


def test_begin_inference_session_begins_inference_session(
        command_handler: CommandHandler,
        session_manager: SessionManager,
        mocker: MockFixture):
    """
    When recieving a command telling the handler to being a inference session,
    it should call the session manager and tell it to begin a inference
    session.
    """
    try:
        session_manager.start_training_session(
            0, ModelSpecification([1], [1], ['mlp']), HyperParams(), 1, True)
        session_manager.save_model(0, './test.pth')
        mocker.spy(session_manager, 'start_inference_session')
        request = """
        {
            "jsonrpc": "2.0",
            "method": "begin_session",
            "param": {
                "model": {
                    "inputs": [1],
                    "outputs": [1],
                    "feature_extractors": ["mlp"]
                },
                "session_id": 1,
                "model_path": "./test.pth",
                "training": false,
                "contexts": 1
            },
            "id": 0
        }
        """
        command_handler.handle_command(request)

        assert session_manager.start_inference_session.call_count == 1
    finally:
        os.remove('./test.pth')


def test_get_action_gets_action(
        command_handler: CommandHandler,
        session_manager: SessionManager,
        mocker: MockFixture):
    """
    When receiving a command to get an action, it should call the get_action
    fuction on the SessionManager.
    """
    session_manager.start_training_session(
        0, ModelSpecification([1], [1], ['mlp']), HyperParams(), 1, True)
    mocker.spy(session_manager, 'get_action')
    request = """
    {
	    "jsonrpc": "2.0",
	    "method": "get_action",
	    "param": {
		    "inputs": [[0.1], [1.0]],
            "context": 0,
            "session_id": 0
		},
	    "id": 0
	}
    """
    command_handler.handle_command(request)

    assert session_manager.get_action.call_count == 1


def test_give_reward_gives_reward(
        command_handler: CommandHandler,
        session_manager: SessionManager,
        mocker: MockFixture):
    """
    When receiving a command to give a reward, it should call the give_reward
    fuction on the SessionManager.
    """
    session_manager.start_training_session(
        0, ModelSpecification([1], [1], ['mlp']), HyperParams(), 1, True)
    mocker.patch.object(session_manager, 'give_reward')
    request = """
    {
	    "jsonrpc": "2.0",
	    "method": "give_reward",
	    "param": {
		    "reward": 1.0,
            "context": 0,
            "session_id": 0
		},
	    "id": 0
	}
    """
    command_handler.handle_command(request)

    assert session_manager.give_reward.call_count == 1


def test_end_session_ends_session(
        command_handler: CommandHandler,
        session_manager: SessionManager,
        mocker: MockFixture):
    """
    When receiving a command to end a session, it should call the end_session
    fuction on the SessionManager.
    """
    session_manager.start_training_session(
        0, ModelSpecification([1], [1], ['mlp']), HyperParams(), 1, True)
    mocker.spy(session_manager, 'end_session')
    request = """
    {
	    "jsonrpc": "2.0",
	    "method": "end_session",
	    "param": {
            "session_id": 0
		},
	    "id": 0
	}
    """
    command_handler.handle_command(request)

    assert session_manager.end_session.call_count == 1


def test_close_connection_begins_waiting_for_next_connection(
        command_handler: CommandHandler):
    """
    When receiving a close_connection message, the server should return a
    response attempting to establish another connection.
    """
    request = """
    {
	    "jsonrpc": "2.0",
	    "method": "close_connection",
	    "param": []
	}
    """
    response = command_handler.handle_command(request)

    assert response == ('New connection')


def test_error_response_on_malformed_command(command_handler: CommandHandler):
    """
    When receiving a bad (malformed) command, the correct error response should
    be returned.
    """
    json = """
    {
        "asd": "2.0",
        "method": "end_session",
	    "param": {
            "session": 0
		},
	    "id": 0
    }
    """
    response = command_handler.handle_command(json)

    assert response == ('{"jsonrpc":"2.0",'
                        '"error":{"code":-32600,"message":"Invalid Request"},'
                        '"id":null}')


def test_error_response_on_bad_json(command_handler: CommandHandler):
    """
    When receiving a bad (invalid json) command, the correct error response
    should be returned.
    """
    json = """
    {
        "asd": "2.", "a]
    }
    """
    response = command_handler.handle_command(json)

    assert response == ('{"jsonrpc":"2.0",'
                        '"error":{"code":-32700,"message":"Parse error"},'
                        '"id":null}')


def test_error_on_invalid_method(command_handler: CommandHandler):
    """
    When receiving a method that doesn't exist, the correct error response
    should be returned.
    """
    json = """
    {
	    "jsonrpc": "2.0",
	    "method": "asd",
	    "param": {
            "session_id": 0
		},
	    "id": 0
	}
    """
    response = command_handler.handle_command(json)

    assert response == ('{"jsonrpc":"2.0",'
                        '"error":{"code":-32601,"message":"Method not found"},'
                        '"id":0}')
