"""
Tests for the Singularity Trainer cloud finish_game function.
"""

# pylint: disable=no-member

import json
from unittest.mock import patch, MagicMock, Mock

import pytest
import requests
import werkzeug.exceptions

from conftest import BASE_URL
import main


@patch('main.db', MagicMock())
def test_finish_game_returns_401_if_no_authentication():
    """
    When no authentication is provided, HTTP status code 401 should be
    returned.
    """
    request = Mock(json={}, headers=Mock(get=Mock(return_value=None)))
    with pytest.raises(werkzeug.exceptions.Unauthorized):
        main.finish_game(request)


@patch('main.db', MagicMock())
def test_finish_game_returns_401_if_bad_authentication():
    """
    When incorrect authentication is provided, HTTP status code 401 should be
    returned.
    """
    (main.db
        .collection.return_value
        .document.return_value
        .get.return_value
        .exists.return_value) = 'sdf'

    request = Mock(json={}, headers=Mock(
        get=Mock(return_value='Bearer asd')))
    with pytest.raises(werkzeug.exceptions.Unauthorized):
        main.finish_game(request)


@patch('main.db', MagicMock())
def test_finish_game_throws_if_username_doesnt_exist():
    """
    When finish_game is called with names that don't exist in the database,
    an error should be raised.
    """
    (main.db
        .collection.return_value
        .document.return_value
        .get.return_value
        .get.return_value) = 'asd'

    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = []

    request = Mock(json={
        'players': ['Bob', 'Steve'],
        'result': -1
    }, headers=Mock(
        get=Mock(return_value='Bearer asd')))
    with pytest.raises(RuntimeError):
        main.finish_game(request)


@patch('main.db', MagicMock())
def test_finish_game_returns_ok():
    """
    When finish_game is called with no errors, it shoudl return successfully.
    """
    (main.db
        .collection.return_value
        .document.return_value
        .get.return_value
        .get.return_value) = 'asd'

    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = [
            MagicMock(id='asd', to_dict=Mock(return_value={'elo': 0}))]

    request = Mock(json={
        'players': ['Bob', 'Steve'],
        'result': 1
    }, headers=Mock(
        get=Mock(return_value='Bearer asd')))

    result = json.loads(main.finish_game(request))

    assert result == {'success': True}


@pytest.mark.integration
def test_finish_game_updates_database(db):
    """
    When finish_game is called correctly, the two users should have their Elos
    updated appropriately and be marked as 'idle'.
    """
    requests.post(BASE_URL + "login", json={
        'username': '__test1'
    })
    requests.post(BASE_URL + "login", json={
        'username': '__test2'
    })

    secret = db.collection('secrets').document('server').get().get('value')

    response = requests.post(BASE_URL + "finish_game", headers={
        'Authorization': f'Bearer {secret}'
    }, json={
        'players': ['__test1', '__test2'],
        'result': 0
    })

    assert response.json()['success']

    users = db.collection('users')
    user_1 = list(users.where('username', '==', '__test1').stream())[0]
    user_2 = list(users.where('username', '==', '__test2').stream())[0]

    assert user_1.to_dict()['elo'] > 0
    assert user_2.to_dict()['elo'] < 0

    assert user_1.to_dict()['status'] == 'idle'
    assert user_2.to_dict()['status'] == 'idle'
