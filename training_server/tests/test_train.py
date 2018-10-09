"""
Tests for train.py
"""
#pylint: disable=W0621
import queue
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
        self.location = torch.randint(-2, 3, (2,)) / 2
        self.reward_location = torch.randint(-2, 3, (2,)) / 2

    def move(self, direction: int):
        """
        Moves the player.
        If the player is out of bounds, reset the game.
        If the player is on the reward location, give a reward then reset the
        game.
        """
        if direction == 0:
            move_direction = torch.Tensor([0.5, 0.])
        elif direction == 1:
            move_direction = torch.Tensor([-0.5, 0.])
        elif direction == 2:
            move_direction = torch.Tensor([0., 0.5])
        elif direction == 3:
            move_direction = torch.Tensor([0., -0.5])
        else:
            move_direction = torch.Tensor([0., 0.])

        self.location += move_direction

        if torch.eq(self.location, self.reward_location).all():
            self.reward += 1
            self.reset()

        if (self.location < -1).any() or (self.location > 1).any():
            self.reset()
            self.reward -= 1

    def get_reward(self) -> float:
        """
        Get a reward from the environment.
        If rewards aren't fetched, they accumulate.
        """
        reward = self.reward
        self.reward = 0
        return reward


class DelayedRewardGame:
    """
    A simple game where every turn the player is presented with two options.
    If they pick the correct option (the same every turn) then N turns later
    they get a reward.
    Used to test that the agent learns from delayed rewards.
    """

    def __init__(self, delay: int = 10):
        self.reward_queue = queue.Queue()
        for _ in range(delay):
            self.reward_queue.put(0)

    def act(self, action: int):
        """
        Take an action.
        """
        self.reward_queue.put(action)

    def get_reward(self) -> float:
        """
        Get a reward.
        """
        return self.reward_queue.get()


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
    np.random.seed(1)
    model = ModelSpecification(
        inputs=[2, 3],
        outputs=[3, 4],
        feature_extractors=['mlp', 'mlp']
    )
    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        minibatch_count=4,
        entropy_coef=0.001,
        discount_factor=0.8,
        clip_factor=0.1,
        epochs=3
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
def test_model_improves_when_trained_basic(session: TrainingSession):
    """
    When trained on a very simple problem that rewards it for picking a
    particular output every time, the model should become more likely to pick
    that output.
    """
    observation = [torch.Tensor([1, 2]), torch.Tensor([1, 2, 3])]

    _, starting_raw_probs, _ = session.model(observation)

    for _ in range(100):
        action, _ = session.get_action(observation, 0)
        reward = 1 if action[0] == 0 else 0
        session.give_reward(reward, 0)

    _, raw_probs, _ = session.model(observation)

    assert raw_probs[0][0][0] > starting_raw_probs[0][0][0]


@pytest.mark.training
def test_model_improves_when_trained_on_multiple_outputs(
        session: TrainingSession):
    """
    When trained on a slightly more complex problem that rewards it for picking
    a particular set of outputs every time, the model should become more likely
    to pick those outputs.
    """
    observation = [torch.Tensor([1, 2]), torch.Tensor([1, 2, 3])]

    _, starting_raw_probs, _ = session.model(observation)

    for _ in range(100):
        action, _ = session.get_action(observation, 0)
        reward = 0
        reward += 1 if action[0] == 0 else 0
        reward += 1 if action[1] == 2 else 0
        session.give_reward(reward, 0)

    _, raw_probs = session.model(observation)

    assert raw_probs[0][0][0] > starting_raw_probs[0][0][0]
    assert raw_probs[1][0][2] > starting_raw_probs[1][0][2]


@pytest.mark.training
def test_model_improves_when_trained_in_multiple_contexts():
    """
    When multiple contexts are used, the model should still improve.
    This doesn't test that the multiple contexts are actually helping training,
    only that it doesn't prevent training entirely.
    """
    torch.manual_seed(1)
    np.random.seed(1)
    model = ModelSpecification(
        inputs=[2, 3],
        outputs=[3, 4],
        feature_extractors=['mlp', 'mlp']
    )
    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        minibatch_length=10,
        minibatch_count=2,
        entropy_coef=0.0001,
        discount_factor=0.95,
        gae=0.96,
        epochs=2,
        clip_factor=0.1
    )
    session = TrainingSession(model, hyperparams, 3)

    observation = [torch.Tensor([1, 2]), torch.Tensor([1, 2, 3])]

    _, starting_raw_probs, _ = session.model(observation)

    for i in range(100):
        action, _ = session.get_action(observation, i % 3)
        reward = 0
        reward += 1 if action[0] == 0 else 0
        reward += 1 if action[1] == 2 else 0
        session.give_reward(reward, i % 3)

    _, raw_probs, _ = session.model(observation)

    assert raw_probs[0][0] > starting_raw_probs[0][0]
    assert raw_probs[1][2] > starting_raw_probs[1][2]


@pytest.mark.training
def test_model_learns_simple_game():
    """
    The model should be able to learn a very simple game where it tries to
    reach a goal.
    """
    np.random.seed(1)
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=[4],
        outputs=[4],
        feature_extractors=['mlp']
    )

    hyperparams = HyperParams(
        learning_rate=0.0001,
        batch_size=20,
        minibatch_length=5,
        minibatch_count=10,
        entropy_coef=0.0001,
        discount_factor=0.95,
        gae=0.96,
        epochs=4,
        clip_factor=0.1
    )

    session = TrainingSession(model, hyperparams, 1)
    rewards = []
    environment = MultiContextGame()

    for _ in range(1000):
        observation = torch.cat((environment.location,
                                 environment.reward_location))
        action, _ = session.get_action([observation], 0)
        environment.move(action[0].item())
        reward = environment.get_reward()
        rewards.append(reward)
        session.give_reward(reward, 0)

    assert np.mean(rewards[:100]) + 0.05 < np.mean(rewards[-100:])


@pytest.mark.training
def test_model_learns_with_multiple_contexts():
    """
    When multiple contexts are used, the model should still learn.
    This doesn't check that the model learns faster than with a single context.
    """
    torch.manual_seed(1)
    np.random.seed(1)
    model = ModelSpecification(
        inputs=[4],
        outputs=[4],
        feature_extractors=['mlp']
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        minibatch_length=5,
        minibatch_count=10,
        entropy_coef=0.0001,
        discount_factor=0.95,
        gae=0.96,
        epochs=3,
    )
    session = TrainingSession(model, hyperparams, 10)

    rewards = []
    environments = [MultiContextGame() for _ in range(10)]

    for i in range(1000):
        observation = torch.cat((environments[i % 10].location,
                                 environments[i % 10].reward_location))
        action, _ = session.get_action([observation], i % 10)
        environments[i % 10].move(action[0].item())
        reward = environments[i % 10].get_reward()
        rewards.append(reward)
        session.give_reward(reward, i % 10)

    # pytest.set_trace()
    assert np.mean(rewards[:100]) + 0.05 < np.mean(rewards[-100:])


@pytest.mark.training
def test_model_learns_with_delayed_rewards():
    """
    The model should be able to learn a simple game with delayed rewards.
    """
    torch.manual_seed(1)
    np.random.seed(1)
    model = ModelSpecification(
        inputs=[1],
        outputs=[2],
        feature_extractors=['mlp']
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=60,
        minibatch_length=20,
        minibatch_count=3,
        entropy_coef=0.0001,
        discount_factor=0.99,
        gae=0.96
    )

    session = TrainingSession(model, hyperparams, 1)
    rewards = []
    environment = DelayedRewardGame(delay=10)

    for _ in range(1000):
        observation = torch.Tensor([1])
        action, _ = session.get_action([observation], 0)
        environment.act(action[0].item())
        reward = environment.get_reward()
        rewards.append(reward)
        session.give_reward(reward, 0)

    assert np.mean(rewards[:100]) + 0.05 < np.mean(rewards[-100:])


@pytest.mark.gpu
@pytest.mark.training
def test_model_learns_with_gpu():
    """
    The model should be able to learn a very simple game where it tries to
    reach a goal.
    """
    np.random.seed(1)
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=[4],
        outputs=[4],
        feature_extractors=['mlp']
    )

    hyperparams = HyperParams(
        learning_rate=0.003,
        batch_size=20,
        minibatch_length=5,
        minibatch_count=10,
        entropy_coef=0.0001,
        discount_factor=0.95,
        gae=0.96,
        epochs=3
    )

    session = TrainingSession(model, hyperparams, 1)
    rewards = []
    environment = MultiContextGame()

    for _ in range(1000):
        observation = torch.cat((environment.location,
                                 environment.reward_location))
        action, _ = session.get_action([observation], 0)
        environment.move(action[0].item())
        reward = environment.get_reward()
        rewards.append(reward)
        session.give_reward(reward, 0)

    assert np.mean(rewards[:100]) + 0.05 < np.mean(rewards[-100:])


@pytest.mark.training
def test_lstm_learns_simple_pattern():
    """
    The model should be able to learn a very simple game where it has to pick
    the option corresponding to the previous input.
    Should require an lstm to learn.
    """
    np.random.seed(1)
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=[2],
        outputs=[2],
        feature_extractors=['mlp']
    )

    hyperparams = HyperParams(
        learning_rate=0.003,
        batch_size=10,
        minibatch_length=3,
        minibatch_count=10,
        entropy_coef=0.0001,
        discount_factor=0.95,
        gae=0.96,
        epochs=3
    )

    session = TrainingSession(model, hyperparams, 1)
    rewards = []

    observations = {
        0: torch.Tensor([1, 0]),
        1: torch.Tensor([0, 1])
    }
    observation = observations[np.random.randint(0, 2)]

    for _ in range(1000):
        last_observation = observation
        observation = observations[np.random.randint(0, 2)]
        action, _ = session.get_action([observation], 0)
        reward = (action[0] == last_observation.argmax()).float()
        rewards.append(reward)
        session.give_reward(reward, 0)

    assert np.mean(rewards[-100:]) > 0.55
