"""
Tests for the AI: Artificial Insentience cloud get_user function.
"""

# pylint: disable=no-member

import json
from unittest.mock import patch, MagicMock, Mock

import pytest
import requests
from werkzeug.exceptions import BadRequest

from conftest import BASE_URL
import main


@patch('main.db', MagicMock())
def test_get_user_returns_400_if_username_isnt_provided():
    """
    When get_user is called with a username that doesn't exist, a 400 error
    code should be returned.
    """
    request = Mock(json={'asd': 'sdf'})
    with pytest.raises(BadRequest):
        main.get_user(request)


@patch('main.db', MagicMock())
def test_get_user_returns_400_if_username_doesnt_exist():
    """
    When get_user is called with a username that doesn't exist, a 400 error
    code should be returned.
    """
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = []

    request = Mock(json={'username': 'test'})
    with pytest.raises(BadRequest):
        main.get_user(request)


@patch('main.db', MagicMock())
def test_get_user_returns_info():
    """
    When get_user is called with no errors, it should return the player's
    information.
    """
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = [
            Mock(id='asd', to_dict=Mock(return_value={
                'elo': 1234,
                'credits': 2345,
                'modules': ['asd', 'sdf']
            }))]

    request = Mock(json={'username': 'test'})
    result = json.loads(main.get_user(request))

    assert result == {
        'elo': 1234,
        'credits': 2345,
        'modules': ['asd', 'sdf']
    }


@pytest.mark.integration
def test_get_user_queries_database(db):
    """
    When get_user is called correctly, it should retrieve the correct
    information from the database.
    """
    requests.post(BASE_URL + "login", json={'username': '__test'})

    users = db.collection('users')
    user = list(users.where('username', '==', '__test').stream())[0]
    users.document(user.id).update({'elo': 2345})
    users.document(user.id).update({'credits': 1100})

    response = requests.post(BASE_URL + "get_user",
                             json={'username': '__test'})

    response_json = response.json()
    assert response_json['elo'] == 2345
    assert response_json['credits'] == 1100
    assert len(response_json['modules']) > 1
