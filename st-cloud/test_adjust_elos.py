"""
Tests for the Singularity Trainer cloud adjust_elos function.
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
def test_adjust_elos_returns_401_if_no_authentication():
    """
    When a game is requested, but no authentication is provided HTTP status
    code 401 should be returned.
    """
    request = Mock(json={}, headers=Mock(get=Mock(return_value=None)))
    with pytest.raises(werkzeug.exceptions.Unauthorized):
        main.adjust_elos(request)


@patch('main.db', MagicMock())
def test_adjust_elos_returns_401_if_bad_authentication():
    """
    When a game is requested, but incorrect authentication is provided, HTTP
    status code 401 should be returned.
    """
    (main.db
        .collection.return_value
        .document.return_value
        .get.return_value
        .exists.return_value) = 'sdf'

    request = Mock(json={}, headers=Mock(
        get=Mock(return_value='Bearer asd')))
    with pytest.raises(werkzeug.exceptions.Unauthorized):
        main.adjust_elos(request)


@patch('main.db', MagicMock())
def test_adjust_elos_throws_if_username_doesnt_exist():
    """
    When adjust_elos is called with names that don't exist in the database,
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
        main.adjust_elos(request)


@patch('main.db', MagicMock())
def test_adjust_elos_returns_ok():
    """
    When adjust_elos is called with no errors, it shoudl return successfully.
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

    result = json.loads(main.adjust_elos(request))

    assert result == {'success': True}


@pytest.mark.integration
def test_adjust_elos_changes_elos(db):
    """
    When a user is waiting for a game, and another calls find_game, they should
    both be marked as in_game, and have their gameserver set appropriately.
    """
    requests.post(BASE_URL + "login", json={
        'username': '__test1'
    })
    requests.post(BASE_URL + "login", json={
        'username': '__test2'
    })

    secret = db.collection('secrets').document('server').get().get('value')

    response = requests.post(BASE_URL + "adjust_elos", headers={
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
