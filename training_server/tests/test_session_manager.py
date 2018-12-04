"""
Tests for session manager
"""
#pylint: disable=W0621
import os
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
        inputs=2,
        outputs=3,
        recurrent=False
    )


def test_start_training_session_instantiates_training_session(
        session_manager: SessionManager,
        model: ModelSpecification):
    """
    After starting a session, the session manager should contain a training
    session.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1)
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


def test_get_actions_calls_correct_session(
        session_manager: SessionManager,
        model: ModelSpecification,
        mocker: MockFixture):
    """
    Calling get_actions on a specified session should call the right session.
    """
    try:
        session_manager.start_training_session(0, model, HyperParams(), 1)
        path = './test.pth'
        session_manager.sessions[0].save_model(path)
        session_manager.start_inference_session(1, model, path, 1)

        mocker.spy(session_manager.sessions[0], 'get_actions')
        mocker.spy(session_manager.sessions[1], 'get_actions')

        inputs = [[1, 2]]
        session_manager.get_actions(0, inputs)
        assert session_manager.sessions[0].get_actions.call_count == 1

        session_manager.get_actions(1, inputs)
        assert session_manager.sessions[1].get_actions.call_count == 1
    finally:
        os.remove(path)


def test_give_rewards_calls_correct_session(
        session_manager: SessionManager,
        model: ModelSpecification,
        mocker: MockFixture):
    """
    Calling give_rewards on a session should call the right session.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1)
    session_manager.start_training_session(1, model, HyperParams(), 1)

    mocker.patch.object(session_manager.sessions[0], 'give_rewards')
    mocker.patch.object(session_manager.sessions[1], 'give_rewards')

    session_manager.give_rewards(0, [1.5], [0])

    assert session_manager.sessions[0].give_rewards.call_count == 1
    assert session_manager.sessions[1].give_rewards.call_count == 0


def test_save_model_calls_save_model(
        session_manager: SessionManager,
        model: ModelSpecification,
        mocker: MockFixture):
    """
    When save_model is called, the relevant method on the correct session
    should be called.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1)
    mocker.patch.object(session_manager.sessions[0], 'save_model')
    session_manager.save_model(0, './test.pth')

    assert session_manager.sessions[0].save_model.call_count == 1


def test_end_session_removes_session(
        session_manager: SessionManager,
        model: ModelSpecification):
    """
    When calling end_session, the session should be removed.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1)
    session_manager.end_session(0)

    assert 0 not in session_manager.sessions
