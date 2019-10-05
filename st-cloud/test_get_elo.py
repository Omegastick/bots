"""
Tests for the Singularity Trainer cloud get_elo function.
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
def test_get_elo_returns_400_if_username_isnt_provided():
    """
    When get_elo is called with a username that doesn't exist, a 400 error code
    should be returned.
    """
    request = Mock(json={'asd': 'sdf'})
    with pytest.raises(BadRequest):
        main.get_elo(request)


@patch('main.db', MagicMock())
def test_get_elo_returns_400_if_username_doesnt_exist():
    """
    When get_elo is called with a username that doesn't exist, a 400 error code
    should be returned.
    """
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = []

    request = Mock(json={'username': 'test'})
    with pytest.raises(BadRequest):
        main.get_elo(request)


@patch('main.db', MagicMock())
def test_get_elo_returns_elo():
    """
    When get_elo is called with no errors, it should return the player's Elo.
    """
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = [
            Mock(id='asd', to_dict=Mock(return_value={'elo': 1234}))]

    request = Mock(json={'username': 'test'})
    result = json.loads(main.get_elo(request))

    assert result == {'elo': 1234}


@pytest.mark.integration
def test_get_elo_queries_database(db):
    """
    When get_elo is called correctly, it should retrieve the correct Elo from
    the database.
    """
    users = db.collection('users')
    users.add({'username': '__test1', 'elo': 2345})

    response = requests.post(BASE_URL + "get_elo",
                             json={'username': '__test1'})

    assert response.json()['elo'] == 2345
