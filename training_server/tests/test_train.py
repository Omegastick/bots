"""
Tests for train.py
"""
#pylint: disable=W0621
import queue
from typing import List
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
        self.location = torch.clamp(
            torch.round(torch.randn(2) * 2) / 2, -1, 1)
        self.reward_location = torch.clamp(
            torch.round(torch.randn(2) * 2) / 2, -1, 1)

    def move(self, direction: List[int]):
        """
        Moves the player.
        If the player is out of bounds, reset the game.
        If the player is on the reward location, give a reward then reset the
        game.
        """
        move_direction = torch.Tensor([0., 0.])
        if direction[0]:
            move_direction += torch.Tensor([0.5, 0.])
        if direction[1]:
            move_direction += torch.Tensor([-0.5, 0.])
        if direction[2]:
            move_direction += torch.Tensor([0., 0.5])
        if direction[3]:
            move_direction += torch.Tensor([0., -0.5])

        self.location += move_direction

        if torch.eq(self.location, self.reward_location).all():
            self.reward += 4
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


class LongTermDependencyGame:
    """
    A game for testing whether the agent's RNN can learn long term
    dependencies.
    Consists of a long corridor like so:
    ##############
    #_##########G#
    #S___________#
    ############T#
    ##############
    Where is is the starting point, G is the goal (reward +1) and T is the trap
    (reward -1). Each round, a tile above or below the starting point is
    cleared at random, and that corresponds to which tile at the end of the
    corridor is going to be the goal and which is the trap.
    Each turn, the agent can pick a direction to move in (NESW) and gets an
    observation of the 3x3 grid around itself.
    """

    def __init__(self, length):
        self.length = length
        self.map = np.ones((5, length + 2))
        self.map[2, 1:-2] = 0
        self.map[1:3, length] = 0
        self.goal = []
        self.position = []
        self.goal = []
        self.punishment = []
        self.reward = 0.
        self.reset()

    def reset(self):
        """
        Reset the map and player.
        """
        self.map = np.ones((5, self.length + 2))
        # Corridor
        self.map[2, 1:-2] = 0
        # T-junction
        self.map[1:4, self.length] = 0
        if np.random.rand() > 0.5:
            self.goal = np.array([1, self.length])
            self.map[1, 1] = 0
            self.punishment = np.array([3, self.length])
        else:
            self.goal = np.array([3, self.length])
            self.map[3, 1] = 0
            self.punishment = np.array([1, self.length])

        self.position = np.array([2, 1])

    def act(self, action: int) -> torch.Tensor:
        """
        Take an action and get an observation.
        Returns a 3x3 tensor of ones and zeros showing the area around the
        player.
        """
        # Convert action to movement vector
        if action == 0:
            direction = [1, 0]
        elif action == 1:
            direction = [0, 1]
        elif action == 2:
            direction = [-1, 0]
        else:
            direction = [0, -1]

        # If the location is valid, move
        if self.map[tuple(self.position + direction)] == 0:
            self.position = self.position + direction

        # If the goal or trap is reached, reset the game and give a reward
        if np.all(self.position == self.goal):
            self.reward += 1
            self.reset()
        elif np.all(self.position == self.punishment):
            self.reward -= 1
            self.reset()

    def get_observation(self):
        """
        Gets an observation of the environment.
        """
        observation = self.map[self.position[0] - 1:self.position[0] + 2,
                               self.position[1]]
        observation = torch.from_numpy(observation)

        return observation.contiguous().float()

    def get_reward(self):
        """
        Get a reward from the environment.
        """
        reward = self.reward
        self.reward = 0
        done = True if reward != 0 else False
        return reward, done


@pytest.fixture
def session():
    """
    Returns a new training session:
    model:
        inputs: 5
        outputs: 3
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
        inputs=5,
        outputs=3,
        recurrent=False
    )
    hyperparams = HyperParams(
        learning_rate=0.0001,
        batch_size=20,
        num_minibatch=4,
        entropy_coef=0.001,
        discount_factor=0.1,
        clip_factor=0.2,
        epochs=3,
        use_gae=False
    )
    return TrainingSession(model, hyperparams, 1)


def test_get_actions_returns_correct_shape(
        session: TrainingSession):
    """
    The list returned by get_action should be the same length as the number of
    outputs dimensions on the model.
    """
    observation = torch.Tensor([[1, 2, 3, 4, 5]])
    actions, _ = session.get_actions(observation)

    assert list(actions.shape) == [1, 3]


@pytest.mark.training
def test_model_improves_when_trained_basic(session: TrainingSession):
    """
    When trained on a very simple problem that rewards it for picking a
    particular output every time, the model should become more likely to pick
    that output.
    """
    observation = torch.Tensor([[1, 2, 3, 4, 5]])

    _, starting_features, _ = session.model.base(observation, None, None)
    starting_probs = session.model.dist(starting_features).probs

    for _ in range(100):
        action, _ = session.get_actions(observation)
        reward = 1 if action[0, 0] == 1 else 0
        session.give_rewards(torch.Tensor([reward]), [0])

    _, features, _ = session.model.base(observation, None, None)
    probs = session.model.dist(features).probs

    assert probs[0][0] > starting_probs[0][0]


@pytest.mark.training
def test_model_improves_when_trained_on_multiple_outputs(
        session: TrainingSession):
    """
    When trained on a slightly more complex problem that rewards it for picking
    a particular set of outputs every time, the model should become more likely
    to pick those outputs.
    """
    observation = torch.Tensor([[1, 2, 3, 4, 5]])

    _, starting_features, _ = session.model.base(observation, None, None)
    starting_probs = session.model.dist(starting_features).probs

    for _ in range(100):
        action, _ = session.get_actions(observation)
        reward = 0
        reward += 1 if action[0, 0] == 1 else 0
        reward += 1 if action[0, 1] == 0 else 0
        session.give_rewards(torch.Tensor([reward]), [0])

    _, features, _ = session.model.base(observation, None, None)
    probs = session.model.dist(features).probs

    assert probs[0][0] > starting_probs[0][0]
    assert probs[0][1] < starting_probs[0][1]


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
        inputs=5,
        outputs=3,
        recurrent=False
    )
    hyperparams = HyperParams(
        learning_rate=0.0001,
        batch_size=20,
        num_minibatch=4,
        entropy_coef=0.001,
        discount_factor=0.1,
        clip_factor=0.2,
        epochs=3,
        use_gae=False
    )
    session = TrainingSession(model, hyperparams, 3)

    observation = torch.Tensor([1, 2, 3, 4, 5])
    observation = torch.stack([observation, observation, observation]
                              )

    _, starting_features, _ = session.model.base(observation, None, None)
    starting_probs = session.model.dist(starting_features).probs

    for _ in range(100):
        action, _ = session.get_actions(observation)
        reward = action[:, 0].unsqueeze(1)
        session.give_rewards(reward, [0, 0, 0])

    _, features, _ = session.model.base(observation, None, None)
    probs = session.model.dist(features).probs

    assert probs[0][0] > starting_probs[0][0]


@pytest.mark.training
def test_model_learns_simple_game():
    """
    The model should be able to learn a very simple game where it tries to
    reach a goal.
    """
    np.random.seed(1)
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=4,
        outputs=4,
        recurrent=False
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        num_minibatch=2,
        entropy_coef=0.001,
        discount_factor=0.9,
        gae=0.96,
        epochs=5,
        clip_factor=0.1
    )

    session = TrainingSession(model, hyperparams, 1)
    rewards = []
    environment = MultiContextGame()

    for _ in range(1000):
        observation = torch.cat((environment.location,
                                 environment.reward_location))
        action, _ = session.get_actions(observation)
        environment.move(action[0])
        reward = environment.get_reward()
        rewards.append(reward)
        session.give_rewards(torch.Tensor([reward]), [0])

    assert np.mean(rewards[:100]) + 0.05 < np.mean(rewards[-100:])


@pytest.mark.training
def test_model_learns_with_multiple_contexts():
    """
    When multiple contexts are used, the model should still learn.
    This doesn't check that the model learns faster than with a single context.
    """
    np.random.seed(1)
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=4,
        outputs=4,
        recurrent=False
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        num_minibatch=2,
        entropy_coef=0.001,
        discount_factor=0.9,
        gae=0.96,
        epochs=5,
        clip_factor=0.1
    )

    session = TrainingSession(model, hyperparams, 3)
    rewards = []
    environments = [MultiContextGame() for _ in range(3)]

    for _ in range(300):
        observation = torch.zeros((3, 4))
        for i, environment in enumerate(environments):
            observation[i] = (torch.cat((environment.location,
                                         environment.reward_location)))
        action, _ = session.get_actions(observation)
        for i, environment in enumerate(environments):
            environment.move(action[i])
        reward = [[environment.get_reward()] for environment in environments]
        rewards.append(reward)
        session.give_rewards(torch.Tensor(reward), [0, 0, 0])

    assert np.mean(rewards[:100]) + 0.05 < np.mean(rewards[-100:])


@pytest.mark.training
def test_model_learns_with_delayed_rewards():
    """
    The model should be able to learn a simple game with delayed rewards.
    """
    np.random.seed(1)
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=1,
        outputs=1,
        recurrent=False
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        num_minibatch=2,
        entropy_coef=0.001,
        discount_factor=0.99,
        gae=0.96,
        epochs=3,
        clip_factor=0.1
    )

    session = TrainingSession(model, hyperparams, 1)
    rewards = []
    environment = DelayedRewardGame(delay=8)

    for _ in range(600):
        observation = torch.Tensor([1])
        action, _ = session.get_actions(observation)
        environment.act(action[0].item())
        reward = environment.get_reward()
        rewards.append(reward)
        session.give_rewards(torch.Tensor([reward]), [0])

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
        inputs=4,
        outputs=4,
        recurrent=False
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        num_minibatch=2,
        entropy_coef=0.001,
        discount_factor=0.9,
        gae=0.96,
        epochs=5,
        clip_factor=0.1,
        use_gpu=True
    )

    session = TrainingSession(model, hyperparams, 1)
    rewards = []
    environment = MultiContextGame()

    for _ in range(300):
        observation = torch.cat((environment.location,
                                 environment.reward_location))
        action, _ = session.get_actions(observation)
        environment.move(action[0])
        reward = environment.get_reward()
        rewards.append(reward)
        session.give_rewards(torch.Tensor([reward]), [0])

    assert np.mean(rewards[:100]) + 0.05 < np.mean(rewards[-100:])


@pytest.mark.training
def test_rnn_learns_simple_pattern():
    """
    The model should be able to learn a very simple game where it has to pick
    the option corresponding to the previous input.
    Should require an RNN to learn.
    """
    np.random.seed(1)
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=1,
        outputs=1
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=12,
        num_minibatch=3,
        entropy_coef=0.0001,
        discount_factor=0.1,
        gae=1.,
        epochs=6,
        clip_factor=0.1
    )

    session = TrainingSession(model, hyperparams, 1)
    rewards = []

    observation = torch.Tensor([1])
    actions = []

    for _ in range(100):
        last_observation = observation
        observation = (observation + 1) % 2
        action, _ = session.get_actions(observation)
        reward = (action == last_observation).float()
        rewards.append(int(reward))
        actions.append(action[0].item())
        session.give_rewards(torch.Tensor([reward]), [0])

    assert np.mean(rewards[-100:]) > 0.6
