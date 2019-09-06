"""
Tests for the Singularity Trainer find game cloud function.
"""

# pylint: disable=no-member

import json
from unittest.mock import patch, MagicMock, Mock

import pytest
import requests

from conftest import BASE_URL
import main


@patch('main.db', MagicMock())
def test_find_game_throws_if_no_authentication():
    """
    When a game is requested, but no authentication is provided an error should
    be raised.
    """
    request = Mock(json={})
    with pytest.raises(RuntimeError):
        main.find_game(request)


@patch('main.db', MagicMock())
def test_find_game_throws_if_bad_token_authentication():
    """
    When a game is requested, but incorrect authentication is provided, an
    error should be raised.
    """
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = []

    request = Mock(json={'token': 'asd'})
    with pytest.raises(RuntimeError):
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

    request = Mock(json={'token': 'asd'})
    response = json.loads(main.find_game(request))

    assert response['status'] == 'waiting_for_game'


@patch('main.db', MagicMock())
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

    request = Mock(json={'token': 'asd'})
    response = json.loads(main.find_game(request))

    assert response['status'] == 'in_game'


@pytest.mark.integration
def test_find_game_marks_users_as_in_game(db):
    """
    When a user is waiting for a game, and another calls find_game, they should
    both be marked as in_game.
    """
    token_1 = requests.post(BASE_URL + "login", json={
        'username': '__test1'
    }).json()['token']
    token_2 = requests.post(BASE_URL + "login", json={
        'username': '__test2'
    }).json()['token']

    assert requests.post(BASE_URL + "find_game", json={
        'token': token_1
    }).json() == {'status': 'waiting_for_game'}
    assert requests.post(BASE_URL + "find_game", json={
        'token': token_2
    }).json() == {'status': 'in_game'}

    users = db.collection('users')
    user_1 = list(users.where('username', '==', '__test1').stream())[0]
    user_2 = list(users.where('username', '==', '__test2').stream())[0]

    assert user_1.to_dict()['status'] == 'in_game'
    assert user_2.to_dict()['status'] == 'in_game'
