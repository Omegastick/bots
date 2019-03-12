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
    Returns a new inference session:

    model:
        inputs: 2
        outputs: 3
        recurrent: True
    contexts: 1
    """
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=2,
        outputs=3,
        recurrent=True
    )

    model_path = './test.pth'

    training_session = TrainingSession(model, HyperParams(), 1)
    training_session.save_model(model_path)

    yield InferenceSession(model, model_path, 1)

    os.remove(model_path)


@pytest.fixture
def multi_context_session():
    """
    Returns a new inference session:

    model:
        inputs: 2
        outputs: 3
        recurrent: True
    contexts: 1
    """
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=2,
        outputs=3,
        recurrent=True
    )

    model_path = './test.pth'

    training_session = TrainingSession(model, HyperParams(), 1)
    training_session.save_model(model_path)

    yield InferenceSession(model, model_path, 3)

    os.remove(model_path)


def test_get_action_returns_correct_number_of_actions(
        session: InferenceSession):
    """
    The list returned by get_action should be the same length as the number of
    outputs dimensions on the model.
    """
    actions, _ = session.get_actions([[1, 2]])
    assert actions.shape == (1, 3)


def test_inference_session_holds_hidden_state(session: InferenceSession):
    """
    When using an RNN with an inference session, the inference session should
    initialize the hidden state to all zeros.
    """
    session.get_actions([[1, 2]])
    assert session.hidden_states.shape == (1, 128)


def test_inference_session_handles_multiple_contexts(
        multi_context_session: InferenceSession):
    """
    When using an RNN with an inference session, the inference session should
    initialize the hidden state to all zeros.
    """
    multi_context_session.get_actions([[1, 2], [3, 4], [5, 6]])
    assert multi_context_session.hidden_states.shape == (3, 128)
