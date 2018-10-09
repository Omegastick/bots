"""
Tests for session manager
"""
#pylint: disable=W0621
import os
import numpy as np
import pytest
from pytest_mock import mocker, MockFixture  # pylint: disable=W0611

from training_server.session_manager import SessionManager
from training_server.model import ModelSpecification
from training_server.train import TrainingSession, HyperParams
from training_server.infer import InferenceSession


@pytest.fixture()
def session_manager():
    """
    Creates a SessionManager.
    """
    return SessionManager()


@pytest.fixture()
def model():
    """
    Creates a ModelSpecification.
    """
    return ModelSpecification(
        inputs=[1, 2],
        outputs=[3, 4],
        feature_extractors=['mlp', 'mlp']
    )


def test_start_training_session_instantiates_training_session(
        session_manager: SessionManager,
        model: ModelSpecification):
    """
    After starting a session, the session manager should contain a training
    session.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1, True)
    assert isinstance(session_manager.sessions[0], TrainingSession)


def test_start_inference_session_instantiates_inference_session(
        session_manager: SessionManager,
        model: ModelSpecification):
    """
    After starting a session, the session manager should contain a training
    session.
    """
    try:
        training_session = TrainingSession(model, HyperParams(), 1)
        path = './test.pth'
        training_session.save_model(path)
        session_manager.start_inference_session(0, model, path, 1)
        assert isinstance(session_manager.sessions[0], InferenceSession)
    finally:
        os.remove(path)


def test_get_action_calls_correct_session(
        session_manager: SessionManager,
        model: ModelSpecification,
        mocker: MockFixture):
    """
    Calling get_action on a specified session should call the right session.
    """
    try:
        session_manager.start_training_session(0, model, HyperParams(), 1,
                                               True)
        path = './test.pth'
        session_manager.sessions[0].save_model(path)
        session_manager.start_inference_session(1, model, path, 1)

        mocker.spy(session_manager.sessions[0], 'get_action')
        mocker.spy(session_manager.sessions[1], 'get_action')

        inputs = [np.array([1]), np.array([1, 2])]
        session_manager.get_action(0, inputs, 0)
        assert session_manager.sessions[0].get_action.call_count == 1

        inputs = [np.array([1]), np.array([1, 2])]
        session_manager.get_action(1, inputs)
        assert session_manager.sessions[1].get_action.call_count == 1
    finally:
        os.remove(path)


def test_give_reward_calls_correct_session(
        session_manager: SessionManager,
        model: ModelSpecification,
        mocker: MockFixture):
    """
    Calling give_reward on a session should call the right session.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1, True)
    session_manager.start_training_session(1, model, HyperParams(), 1, True)

    mocker.patch.object(session_manager.sessions[0], 'give_reward')
    mocker.patch.object(session_manager.sessions[1], 'give_reward')

    session_manager.give_reward(0, 1.5, 0)

    assert session_manager.sessions[0].give_reward.call_count == 1
    assert session_manager.sessions[1].give_reward.call_count == 0


def test_save_model_calls_save_model(
        session_manager: SessionManager,
        model: ModelSpecification,
        mocker: MockFixture):
    """
    When save_model is called, the relevant method on the correct session
    should be called.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1, True)
    mocker.patch.object(session_manager.sessions[0], 'save_model')
    session_manager.save_model(0, './test.pth')

    assert session_manager.sessions[0].save_model.call_count == 1


def test_end_session_removes_session(
        session_manager: SessionManager,
        model: ModelSpecification):
    """
    When calling end_session, the session should be removed.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1, True)
    session_manager.end_session(0)

    assert 0 not in session_manager.sessions
