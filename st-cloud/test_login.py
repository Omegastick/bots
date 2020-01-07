"""
Tests for the AI: Artificial Insentience cloud login function.
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
def test_login_gets_user_if_user_exists():
    """
    When logging in with an existing name, that user should be retrieved from
    the database.
    """
    user = MagicMock()
    user.id = 'test'
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = [user]

    request = Mock(json={'username': 'asd'})
    response = main.login(request)
    assert main.db.collection.return_value.document.called
    assert main.db.collection.return_value.document.call_args[0][0] == 'test'
    assert 'token' in json.loads(response)


@patch('main.db', MagicMock())
def test_login_throws_if_multiple_users_exists():
    """
    When logging in with a name that exists twice, and error should be thrown.
    """
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = [MagicMock(), MagicMock()]

    request = Mock(json={'username': 'asd'})
    with pytest.raises(RuntimeError):
        main.login(request)


def test_login_throws_if_no_username_provided():
    """
    When a request is sent without a username field in the Json body, an error
    should be returned.
    """
    request = Mock(json={})
    with pytest.raises(werkzeug.exceptions.BadRequest):
        main.login(request)


@pytest.mark.integration
def test_login_creates_new_user_on_new_login(db):
    """
    When logging in with an unused name, a new user should be created.
    """
    response = requests.post(BASE_URL + "login", json={
        'username': '__test1'
    })

    assert 'token' in response.json()

    users = db.collection('users')

    matching_users = list(users.where('username', '==', '__test1').stream())
    assert len(matching_users) == 1
    assert matching_users[0].to_dict()['elo'] == 0
    assert matching_users[0].to_dict()['credits'] == 0
    assert matching_users[0].to_dict()['modules'] == [
        "base_module", "gun_module", "thruster_module", "laser_sensor_module"]
