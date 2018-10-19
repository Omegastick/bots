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
        feature_extractors=['mlp', 'mlp'],
        recurrent=False
    )
    hyperparams = HyperParams(
        learning_rate=0.0001,
        batch_size=20,
        entropy_coef=0.001,
        discount_factor=0.1,
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

    _, starting_raw_probs = session.model(observation)

    for _ in range(100):
        action, _ = session.get_action(observation, 0)
        reward = 1 if action[0] == 0 else 0
        session.give_reward(reward, 0)

    _, raw_probs = session.model(observation)

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

    _, starting_raw_probs = session.model(observation)

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
        feature_extractors=['mlp', 'mlp'],
        recurrent=False
    )
    hyperparams = HyperParams(
        learning_rate=0.0001,
        batch_size=6,
        minibatch_length=3,
        entropy_coef=0.0001,
        discount_factor=0.1,
        gae=0.96,
        epochs=3,
        clip_factor=0.1
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

    assert raw_probs[0][0][0] > starting_raw_probs[0][0][0]
    assert raw_probs[1][0][2] > starting_raw_probs[1][0][2]


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
        feature_extractors=['mlp'],
        recurrent=False
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        minibatch_length=10,
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
        feature_extractors=['mlp'],
        recurrent=False
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        minibatch_length=10,
        entropy_coef=0.001,
        discount_factor=0.9,
        gae=0.96,
        epochs=5,
        clip_factor=0.1
    )
    session = TrainingSession(model, hyperparams, 3)

    rewards = []
    environments = [MultiContextGame() for _ in range(3)]

    for i in range(2000):
        observation = torch.cat((environments[i % 3].location,
                                 environments[i % 3].reward_location))
        action, _ = session.get_action([observation], i % 3)
        environments[i % 3].move(action[0].item())
        reward = environments[i % 3].get_reward()
        rewards.append(reward)
        session.give_reward(reward, i % 3)

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
        feature_extractors=['mlp'],
        recurrent=False
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=60,
        minibatch_length=20,
        entropy_coef=0.001,
        discount_factor=0.97,
        epochs=5,
        gae=1.
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
def test_cnn_model_learns_simple_game():
    """
    The model should be able to learn a very simple game where it tries to
    reach a goal.
    """
    np.random.seed(1)
    torch.manual_seed(1)
    model = ModelSpecification(
        inputs=[[5, 5]],
        outputs=[4],
        feature_extractors=['cnn'],
        recurrent=False
    )

    hyperparams = HyperParams(
        learning_rate=0.001,
        batch_size=20,
        minibatch_length=10,
        entropy_coef=0.001,
        discount_factor=0.9,
        gae=0.96,
        epochs=5,
        clip_factor=0.1
    )

    session = TrainingSession(model, hyperparams, 1)
    rewards = []
    environment = MultiContextGame()

    def get_observation(environment: MultiContextGame) -> torch.Tensor:
        observation = torch.zeros(5, 5)
        player_location = (environment.location * 2) + 2
        reward_location = (environment.reward_location * 2) + 2
        observation[player_location[0], player_location[1]] = 1
        observation[reward_location[0], reward_location[1]] = -1

    for _ in range(1000):
        observation = [get_observation(environment)]
        action, _ = session.get_action(observation, 0)
        environment.move(action[0].item())
        reward = environment.get_reward()
        rewards.append(reward)
        session.give_reward(reward, 0)

    assert np.mean(rewards[:100]) + 0.05 < np.mean(rewards[-100:])
