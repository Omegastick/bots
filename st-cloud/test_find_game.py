"""
Tests for the Singularity Trainer find game cloud function.
"""

# pylint: disable=no-member

import json
import re
from unittest.mock import patch, MagicMock, Mock

import pytest
import requests
import werkzeug.exceptions

from conftest import BASE_URL
import main

UNALLOCATED_GAMESERVER = {
    'kind': 'GameServerAllocation',
    'apiVersion': 'v1',
    'metadata': {
        'namespace': 'default',
        'creationTimestamp': '2019-09-09T08:48:55Z'
    },
    'spec': {
        'multiClusterSetting': {
            'policySelector': {}
        },
        'required': {
            'matchLabels': {
                'agones.dev/fleet': 'singularity-trainer'
            }
        },
        'scheduling': 'Packed',
        'metadata': {}
    },
    'status': {
        'state': 'UnAllocated',
        'gameServerName': ''
    }
}


ALLOCATED_GAMESERVER = {
    'kind': 'GameServerAllocation',
    'apiVersion': 'v1',
    'metadata': {
        'name': 'singularity-trainer-sv5mj-wq4dp',
        'namespace': 'default',
        'creationTimestamp': '2019-09-09T08:50:28Z'
    },
    'spec': {
        'multiClusterSetting': {
            'policySelector': {}
        },
        'required': {
            'matchLabels': {
                'agones.dev/fleet': 'singularity-trainer'
            }
        },
        'scheduling': 'Packed',
        'metadata': {}
    },
    'status': {
        'state': 'Allocated',
        'gameServerName': 'singularity-trainer-sv5mj-wq4dp',
        'ports': [{'name': 'default', 'port': 7693}],
        'address': '34.84.249.148',
        'nodeName': 'gke-st-dev-default-pool-add6043f-llk4'
    }
}


@patch('main.db', MagicMock())
def test_find_game_returns_401_if_no_authentication():
    """
    When a game is requested, but no authentication is provided HTTP status
    code 401 should be returned.
    """
    request = Mock(json={}, headers=Mock(get=Mock(return_value=None)))
    with pytest.raises(werkzeug.exceptions.Unauthorized):
        main.find_game(request)


@patch('main.db', MagicMock())
def test_find_game_returns_401_if_bad_authentication():
    """
    When a game is requested, but incorrect authentication is provided HTTP
    status code 401 should be returned.
    """
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = []

    request = Mock(json={}, headers=Mock(
        get=Mock(return_value='Bearer asd')))
    with pytest.raises(werkzeug.exceptions.Unauthorized):
        main.find_game(request)


@patch('main.db', MagicMock())
def test_find_game_returns_waiting_if_no_other_players_are_ready():
    """
    When a game is requested with no other waiting users, it should tell the
    user that it is waiting for a game.
    """
    def where(field, *_, **__):
        """
        Side effect for db.collection('users').where(...).stream()
        """
        if field == 'token':
            mock = Mock()
            mock.stream.return_value = [MagicMock()]
            return mock
        if field == 'status':
            mock = Mock()
            mock.stream.return_value = []
            return mock
        raise ValueError()

    (main.db
        .collection.return_value
        .where.side_effect) = where

    request = Mock(json={}, headers=Mock(
        get=Mock(return_value='Bearer asd')))
    response = json.loads(main.find_game(request))

    assert response['status'] == 'waiting_for_game'


@patch('main.db', MagicMock())
@patch('main.allocate_gameserver', Mock(return_value=ALLOCATED_GAMESERVER))
def test_find_game_returns_in_game_if_other_players_are_waiting():
    """
    When a game is requested while another user is waiting, it should tell the
    user they have found a game.
    """
    def where(field, *_, **__):
        """
        Side effect for db.collection('users').where(...).stream()
        """
        if field == 'token':
            mock = Mock()
            mock.stream.return_value = [MagicMock(id='asd')]
            return mock
        if field == 'status':
            mock = Mock()
            mock.stream.return_value = [MagicMock(id='sdf')]
            return mock
        raise ValueError()

    (main.db
        .collection.return_value
        .where.side_effect) = where

    request = Mock(json={}, headers=Mock(
        get=Mock(return_value='Bearer asd')))
    response = json.loads(main.find_game(request))

    assert response['status'] == 'in_game'


@patch('main.db', MagicMock())
@patch('main.allocate_gameserver', Mock(return_value=UNALLOCATED_GAMESERVER))
def test_find_game_returns_waiting_if_no_gameservers_are_ready():
    """
    When a game is requested with no gameservers available, it should tell the
    user that it is waiting for a game.
    """
    def where(field, *_, **__):
        """
        Side effect for db.collection('users').where(...).stream()
        """
        if field == 'token':
            mock = Mock()
            mock.stream.return_value = [MagicMock(id='asd')]
            return mock
        if field == 'status':
            mock = Mock()
            mock.stream.return_value = [MagicMock(id='sdf')]
            return mock
        raise ValueError()

    (main.db
        .collection.return_value
        .where.side_effect) = where

    request = Mock(json={}, headers=Mock(
        get=Mock(return_value='Bearer asd')))
    response = json.loads(main.find_game(request))

    assert response['status'] == 'waiting_for_game'


@pytest.mark.integration
def test_find_game_updates_database(db):
    """
    When a user is waiting for a game, and another calls find_game, they should
    both be marked as in_game, and have their gameserver set appropriately.
    """
    token_1 = requests.post(BASE_URL + "login", json={
        'username': '__test1'
    }).json()['token']
    token_2 = requests.post(BASE_URL + "login", json={
        'username': '__test2'
    }).json()['token']

    assert requests.post(BASE_URL + "find_game", headers={
        'Authorization': f'Bearer {token_1}'
    }).json()['status'] == 'waiting_for_game'
    assert requests.post(BASE_URL + "find_game", headers={
        'Authorization': f'Bearer {token_2}'
    }).json()['status'] == 'in_game'

    users = db.collection('users')
    user_1 = list(users.where('username', '==', '__test1').stream())[0]
    user_2 = list(users.where('username', '==', '__test2').stream())[0]

    assert user_1.to_dict()['status'] == 'in_game'
    assert user_2.to_dict()['status'] == 'in_game'
    assert re.match(r"^tcp:\/\/", user_1.to_dict()['gameserver'])
    assert re.match(r"^tcp:\/\/", user_2.to_dict()['gameserver'])
