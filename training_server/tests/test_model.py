"""
Tests for model.py
"""
#pylint: disable=W0621
import pytest
import torch

from training_server.model import CustomPolicy


@pytest.fixture
def model():
    """
    Returns a new, randomly parameterised model.
    Inputs: 9
    Outputs: 5
    """
    return CustomPolicy(9, 5)
