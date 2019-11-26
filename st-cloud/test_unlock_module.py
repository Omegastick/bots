"""
Tests for the Singularity Trainer cloud unlock_module function.
"""

# pylint: disable=no-member

from unittest.mock import patch, MagicMock, Mock

import json
import pytest
import requests
import werkzeug.exceptions

from conftest import BASE_URL
import main


@patch('main.db', MagicMock())
def test_unlock_module_returns_401_if_no_authentication():
    """
    When an attempt to unlock a module is made, but no authentication is
    provided, HTTP status code 401 should be returned.
    """
    request = Mock(json={}, headers=Mock(get=Mock(return_value=None)))
    with pytest.raises(werkzeug.exceptions.Unauthorized):
        main.unlock_module(request)


@patch('main.db', MagicMock())
def test_unlock_module_returns_401_if_bad_authentication():
    """
    When an attempt to unlock a module is made, but incorrect authentication is
    provided, HTTP status code 401 should be returned.
    """
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = []

    request = Mock(json={}, headers=Mock(
        get=Mock(return_value='Bearer asd')))
    with pytest.raises(werkzeug.exceptions.Unauthorized):
        main.unlock_module(request)


@patch('main.db', MagicMock())
def test_unlock_module_returns_failure_if_cant_afford_module():
    """
    When an attempt to unlock a module is made, but they player can't afford
    the module requested, a failure object should be returned.
    """
    def collection(field, *_, **__):
        """
        Side effect for db.collection(...)
        """
        if field == 'users':
            mock = MagicMock()
            (mock.where.return_value
                 .stream.return_value) = [
                     Mock(id='asd', to_dict=Mock(return_value={
                         'credits': 0
                     }))
            ]
            return mock
        if field == 'modules':
            mock = MagicMock()
            (mock.where.return_value
                 .stream.return_value) = [
                MagicMock(to_dict=Mock(return_value={
                    'name': 'square_hull',
                    'price': 1000
                }))
            ]
            return mock
        raise ValueError()

    main.db.collection.side_effect = collection

    request = Mock(json={'module': 'square_hull'}, headers=Mock(
        get=Mock(return_value='Bearer asd')))

    with pytest.raises(werkzeug.exceptions.BadRequest):
        main.unlock_module(request)


@patch('main.db', MagicMock())
def test_unlock_module_returns_failure_if_invalid_module():
    """
    When an attempt to unlock a module is made, but incorrect authentication is
    provided, HTTP status code 401 should be returned.
    """
    def collection(field, *_, **__):
        """
        Side effect for db.collection(...)
        """
        if field == 'users':
            mock = MagicMock()
            (mock.where.return_value
                 .stream.return_value) = [
                     Mock(id='asd',
                          to_dict=Mock(
                              return_value={
                                  'credits': 1000
                              }
                          ))
            ]
            return mock
        if field == 'modules':
            mock = MagicMock()
            mock.stream.return_value = []
            return mock
        raise ValueError()

    main.db.collection.side_effect = collection

    request = Mock(json={'module': ''}, headers=Mock(
        get=Mock(return_value='Bearer asd')))

    with pytest.raises(werkzeug.exceptions.BadRequest):
        main.unlock_module(request)


@patch('main.db', MagicMock())
def test_unlock_module_returns_success():
    """
    When an attempt to unlock a module is made, and everything is alright, a
    success object should be returned.
    """
    def collection(field, *_, **__):
        """
        Side effect for db.collection(...)
        """
        if field == 'users':
            mock = MagicMock()
            (mock.where.return_value
                 .stream.return_value) = [
                     Mock(id='asd', to_dict=Mock(return_value={
                         'credits': 1100
                     }))
            ]
            return mock
        if field == 'modules':
            mock = MagicMock()
            (mock.where.return_value
                 .stream.return_value) = [
                MagicMock(to_dict=Mock(return_value={
                    'name': 'square_hull',
                    'price': 1000
                }))
            ]
            return mock
        raise ValueError()

    main.db.collection.side_effect = collection

    request = Mock(json={'module': 'square_hull'}, headers=Mock(
        get=Mock(return_value='Bearer asd')))
    result = main.unlock_module(request)

    assert json.loads(result) == {'success': True}


@pytest.mark.integration
def test_find_game_updates_database(db):
    """
    When a user is waiting for a game, and another calls find_game, they should
    both be marked as in_game, and have their gameserver set appropriately.
    """
    token = requests.post(BASE_URL + "login",
                          json={'username': '__test'}).json()['token']

    users = db.collection('users')
    user = list(users.where('username', '==', '__test').stream())[0]
    users.document(user.id).update({'credits': 1100})

    assert requests.post(
        BASE_URL + "unlock_module",
        headers={
            'Authorization': f'Bearer {token}'},
        json={'module': 'square_hull'}
    ).json()['success']

    user = list(users.where('username', '==', '__test').stream())[0]

    assert "square_hull" in user.to_dict()['modules']
    assert user.to_dict()['credits'] == 100
