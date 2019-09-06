"""
Tests for the Singularity Trainer cloud login function.
"""

import json
from unittest.mock import patch, MagicMock, Mock

import pytest

import main


@patch('main.db', MagicMock())
def test_login_creates_new_user_on_new_login():
    """
    When logging in with an unused name, a new user should be created.
    """
    (main.db
        .collection.return_value
        .where.return_value
        .stream.return_value) = []

    request = Mock(json={'username': 'asd'})
    response = main.login(request)
    assert main.db.collection.return_value.add.called
    assert 'token' in json.loads(response)


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
