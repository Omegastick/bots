"""
Tests for model.py
"""
#pylint: disable=W0621
from training_server.model import CustomPolicy


def test_recurrent_policy_creates_correctly():
    """
    When creating a recurrent policy, its is_recurrent parameter should return
    True.
    """
    policy = CustomPolicy(2, 2, True)

    assert policy.is_recurrent


def test_recurrent_hidden_state_size_is_specified_correctly():
    """
    When creating a recurrent policy, its recurrent_hidden_state_size should
    match the specified hidden_size.
    """
    policy = CustomPolicy(2, 2, True, 256)

    assert policy.recurrent_hidden_state_size == 256
