"""
Tests for infer.py
"""
#pylint: disable=W0621
import os
import pytest
import torch

from training_server.infer import InferenceSession
from training_server.train import TrainingSession, HyperParams
from training_server.model import ModelSpecification


@pytest.fixture
def session():
    """
    Returns a new training session:
    model:
        inputs: 2
        outputs: 3
    hyperparams:
        learning_rate: 0.0001
        batch_size: 20
        entropy_coef: 0.001
        discount_factor: 0.8
    contexts: 1
    """
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=2,
        outputs=3
    )

    model_path = './test.pth'

    training_session = TrainingSession(model, HyperParams(), 1)
    training_session.save_model(model_path)

    yield InferenceSession(model, model_path, 1)

    os.remove(model_path)


def test_get_action_returns_correct_number_of_actions(
        session: InferenceSession):
    """
    The list returned by get_action should be the same length as the number of
    outputs dimensions on the model.
    """
    observation = torch.Tensor([1, 2])
    actions, _ = session.get_actions(observation)
    expected = 3
    assert len(actions) == expected
