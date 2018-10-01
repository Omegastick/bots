"""
Tests for train.py
"""
#pylint: disable=W0621
import pytest
import torch
import numpy as np

from training_server.train import TrainingSession, HyperParams
from training_server.model import ModelSpecification


class MultiContextGame:
    """
    Quick toy environment where agents try to get to a randomly decided reward
    location.
    Should learn faster with multiple agents.
    """

    def __init__(self):
        self.location = None
        self.reward_location = None
        self.reset()
        self.reward = 0

    def reset(self):
        """
        Resets the game.
        """
        self.location = torch.randint(-10, 10, (2,)) / 10
        self.reward_location = torch.randint(-10, 10, (2,)) / 10

    def move(self, direction: int):
        """
        Moves the player.
        If the player is out of bounds, reset the game.
        If the player is on the reward location, give a reward then reset the
        game.
        """
        if direction == 0:
            move_direction = torch.Tensor([0.1, 0.])
        elif direction == 1:
            move_direction = torch.Tensor([-0.1, 0.])
        elif direction == 2:
            move_direction = torch.Tensor([0., 0.1])
        elif direction == 3:
            move_direction = torch.Tensor([0., -0.1])
        else:
            move_direction = torch.Tensor([0., 0.])

        self.location += move_direction

        if torch.eq(self.location, self.reward_location).all():
            self.reward += 1
            self.reset()

        for coordinate in self.location:
            if coordinate > 1 or coordinate < -1:
                self.reset()
                self.reward -= -1

    def get_reward(self):
        """
        Get a reward from the environment.
        If rewards aren't fetched, they accumulate.
        """
        reward = self.reward
        self.reward = 0
        return reward


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
        entropy_coef: 0.001
        discount_factor: 0.8
    contexts: 1
    """
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=[2, 3],
        outputs=[3, 4],
        feature_extractors=['mlp', 'mlp']
    )
    hyperparams = HyperParams(
        learning_rate=0.0001,
        batch_size=20,
        entropy_coef=0.001,
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


@pytest.mark.training
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


@pytest.mark.training
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


@pytest.mark.training
def test_model_improves_when_trained_in_multiple_contexts():
    """
    When multiple contexts are used, the model should still improve.
    This doesn't test that the multiple contexts are actually helping training,
    only that it doesn't prevent training entirely.
    """
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=[2, 3],
        outputs=[3, 4],
        feature_extractors=['mlp', 'mlp']
    )
    hyperparams = HyperParams(
        learning_rate=0.0001,
        batch_size=20,
        entropy_coef=0.001,
        discount_factor=0.8
    )
    session = TrainingSession(model, hyperparams, 3)

    observation = [torch.Tensor([1, 2]), torch.Tensor([1, 2, 3])]

    _, starting_raw_probs = session.model(observation)

    for i in range(100):
        action, _ = session.get_action(observation, i % 3)
        reward = 0
        reward += 1 if action[0] == 0 else 0
        reward += 1 if action[1] == 2 else 0
        session.give_reward(reward, i % 3)

    _, raw_probs = session.model(observation)

    assert raw_probs[0][0] > starting_raw_probs[0][0]
    assert raw_probs[1][2] > starting_raw_probs[1][2]


@pytest.mark.training
def test_model_multiple_contexts_improve_training():
    """
    When multiple contexts are used, the model should do better on some
    environments than with a single environment.
    """
    # torch.manual_seed(1)
    model = ModelSpecification(
        inputs=[4],
        outputs=[4],
        feature_extractors=['mlp']
    )

    # Single context
    hyperparams = HyperParams(
        learning_rate=0.0001,
        batch_size=5,
        entropy_coef=0.001,
        discount_factor=0.95
    )
    single_session = TrainingSession(model, hyperparams, 1)

    single_rewards = []
    single_environment = MultiContextGame()

    for _ in range(1000):
        observation = torch.cat((single_environment.location,
                                 single_environment.reward_location))
        action, _ = single_session.get_action([observation], 0)
        single_environment.move(action[0].item())
        reward = single_environment.get_reward()
        single_rewards.append(reward)
        single_session.give_reward(reward, 0)

    # Multiple contexts
    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=5,
        entropy_coef=0.001,
        discount_factor=0.95
    )
    multi_session = TrainingSession(model, hyperparams, 10)

    multi_rewards = []
    environments = [MultiContextGame() for _ in range(10)]

    for i in range(1000):
        observation = torch.cat((environments[i % 10].location,
                                 environments[i % 10].reward_location))
        action, _ = multi_session.get_action([observation], 0)
        environments[i % 10].move(action[0].item())
        reward = environments[i % 10].get_reward()
        multi_rewards.append(reward)
        multi_session.give_reward(reward, 0)

    # pytest.set_trace()
    assert np.mean(multi_rewards) > np.mean(single_rewards)
