"""
Tests for the AI: Artificial Insentience cloud get_all_modules function.
"""

# pylint: disable=no-member

import json
from unittest.mock import patch, MagicMock, Mock

import pytest
import requests

from conftest import BASE_URL
import main


@patch('main.db', MagicMock())
def test_get_all_modules_returns_info():
    """
    When get_all_modules is called with no errors, it should return a list of
    all modules and their prices.
    """
    (main.db
        .collection.return_value
        .stream.return_value) = [
        Mock(to_dict=Mock(return_value={
            'name': 'asd',
            'price': 123
        })),
        Mock(to_dict=Mock(return_value={
            'name': 'sdf',
            'price': 234
        }))
    ]

    request = Mock(json={'username': 'test'})
    result = json.loads(main.get_all_modules(request))

    assert result == {
        'modules': [
            {
                'name': 'asd',
                'price': 123
            },
            {
                'name': 'sdf',
                'price': 234
            }
        ]
    }


@pytest.mark.integration
def test_get_all_modules_queries_database():
    """
    When get_all_modules is called correctly, it should retrieve the correct
    information from the database.
    """
    response = requests.post(BASE_URL + 'get_all_modules')

    response_json = response.json()
    for module in response_json['modules']:
        assert 'name' in module
        assert 'price' in module
