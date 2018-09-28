"""
Tests for train.py.
"""
#pylint: disable=W0621
import pytest
import torch

from ..train import TrainingSession, ModelSpecification, HyperParams


@pytest.fixture
def session():
    """
    Returns a new training session:
    model:
        inputs: 2, 3
        outputs: 3, 4
        features: mlp, mlp
    hyperparams:
        learning_rate: 0.0001
        batch_size: 20
        entropy_coef: 0.1
        discount_factor: 0.8
    contexts: 1
    """
    model = ModelSpecification(
        inputs=[2, 3],
        outputs=[3, 4],
        feature_extractors=['mlp', 'mlp']
    )
    hyperparams = HyperParams(
        learning_rate=0.0001,
        batch_size=20,
        entropy_coef=0.1,
        discount_factor=0.8
    )
    return TrainingSession(model, hyperparams, 1)


def test_get_action_returns_correct_number_of_actions(
        session: TrainingSession):
    """
    The list returned by get_action should be the same length as the number of
    outputs dimensions on the model.
    """
    observation = [torch.Tensor([1, 2]), torch.Tensor([1, 2, 3])]
    actions, _ = session.get_action(observation, 0)
    expected = 2
    assert len(actions) == expected


def test_model_improves_when_trained(session: TrainingSession):
    """
    When trained on a very simple problem that rewards it for picking a
    particular output every time, the model should become more likely to pick
    that output.
    """
    observation = [torch.Tensor([1, 2]), torch.Tensor([1, 2, 3])]

    _, starting_raw_probs = session.model(observation)

    for _ in range(100):
        action, _ = session.get_action(observation, 0)
        reward = 1 if action[0] == 0 else 0
        session.give_reward(reward, 0)

    _, raw_probs = session.model(observation)

    assert raw_probs[0][0] > starting_raw_probs[0][0]


def test_model_improves_when_trained_on_multiple_outputs(
        session: TrainingSession):
    """
    When trained on a slightly more complex problem that rewards it for picking
    a particular set of outputs every time, the model should become more likely
    to pick those outputs.
    """
    observation = [torch.Tensor([1, 2]), torch.Tensor([1, 2, 3])]

    _, starting_raw_probs = session.model(observation)

    for _ in range(100):
        action, _ = session.get_action(observation, 0)
        reward = 0
        reward += 1 if action[0] == 0 else 0
        reward += 1 if action[1] == 2 else 0
        session.give_reward(reward, 0)

    _, raw_probs = session.model(observation)

    assert raw_probs[0][0] > starting_raw_probs[0][0]
    assert raw_probs[1][2] > starting_raw_probs[1][2]
