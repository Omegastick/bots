"""
Tests for session manager.
"""
#pylint: disable=W0621
import os
import torch
import pytest
from pytest_mock import mocker

from ..session_manager import SessionManager
from ..model import ModelSpecification
from ..train import TrainingSession, HyperParams


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
        model: ModelSpecification
):
    """
    After starting a session, the session manager should contain a training
    session.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1, True)
    assert isinstance(session_manager.sessions[0], TrainingSession)


def test_start_inference_session_instantiates_inference_session(
        session_manager: SessionManager,
        model: ModelSpecification
):
    """
    After starting a session, the session manager should contain a training
    session.
    """
    try:
        training_session = TrainingSession(model, HyperParams(), 1)
        path = './test.pth'
        training_session.save_model(path)
        session_manager.start_inference_session(
            session_id=0, model=model, path=path)
        assert isinstance(session_manager.sessions[0], TrainingSession)
    finally:
        os.remove(path)


def test_get_action_calls_correct_session(
    session_manager: SessionManager,
    model: ModelSpecification
):
    """
    Calling get_action on a specified session should call the right session.
    """
    try:
        session_manager.start_training_session(
            0, model, HyperParams(), 1, True)
        path = './test.pth'
        session_manager.sessions[0].save_model(path)
        session_manager.start_inference_session(
            session_id=1, model=model, path=path)

        mocker.spy(session_manager.sessions[0], 'get_action')
        mocker.spy(session_manager.sessions[1], 'get_action')

        inputs = [torch.Tensor([1]), torch.Tensor([1, 2])]
        session_manager.get_action(0, inputs, 0)
        assert session_manager.sessions[0].call_count == 1

        session_manager.get_action(1, inputs, 0)
        assert session_manager.sessions[1].call_count == 1
    finally:
        os.remove(path)


def test_give_reward_calls_correct_session(
    session_manager: SessionManager,
    model: ModelSpecification
):
    """
    Calling give_reward on a session should call the right session.
    """
    session_manager.start_training_session(0, model, HyperParams(), 1, True)
    session_manager.start_training_session(1, model, HyperParams(), 1, True)

    mocker.patch.object(session_manager.sessions[0].give_reward)
    mocker.patch.object(session_manager.sessions[1].give_reward)

    session_manager.give_reward(0, 1.5)

    assert session_manager.sessions[0].call_count == 1
    assert session_manager.sessions[1].call_count == 0
