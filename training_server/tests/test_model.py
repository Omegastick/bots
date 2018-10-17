"""
Tests for model.py
"""
#pylint: disable=W0621
import pytest
import torch

from training_server.model import Model, ModelSpecification


@pytest.fixture
def model():
    """
    Returns a new, randomly parameterised model.
    Inputs: 3, 6
    Outputs: 2, 10
    Features: mlp, mlp
    """
    return Model([3, 6], [2, 10], ['mlp', 'mlp'])


def test_model_raw_prob_dimensions(model: ModelSpecification):
    """
    The outputted raw probabilities should match dimensions specified in the
    constructor.
    """
    observation = [torch.Tensor([1, 2, 3]), torch.Tensor([1, 2, 3, 4, 5, 6])]
    _, raw_probs = model(observation)

    expected = [2, 10]
    dimensions = [len(x[0]) for x in raw_probs]

    assert dimensions == expected


def test_model_prob_dimensions(model: ModelSpecification):
    """
    The outputted probabilities should match dimensions specified in the
    constructor.
    """
    observation = [torch.Tensor([1, 2, 3]), torch.Tensor([1, 2, 3, 4, 5, 6])]
    _, probs, log_probs = model.act(observation)

    expected = [2, 10]
    dimensions = [len(x) for x in probs]
    log_dimensions = [len(x) for x in log_probs]

    assert dimensions == expected
    assert log_dimensions == expected


def test_model_backprop(model: ModelSpecification):
    """
    When backprop and an optimizer is used, the model should train.
    """
    optimizer = torch.optim.Adam(model.parameters())
    observation = [torch.Tensor([1, 2, 3]), torch.Tensor([1, 2, 3, 4, 5, 6])]

    starting_value, starting_probs, _ = model.act(observation)
    starting_reward = sum([prob[0] for prob in starting_probs])

    for _ in range(100):
        value, probs, log_probs = model.act(observation)
        reward = sum([prob[0] for prob in probs])

        value_loss = (reward - value).pow(2)

        actor_loss = -sum([log_prob[0] for log_prob in log_probs])

        optimizer.zero_grad()
        (value_loss + actor_loss).backward()
        optimizer.step()

    assert starting_probs[0][0] < probs[0][0]
    assert abs(starting_reward - starting_value) > abs(reward - value)


def test_probs_add_to_1(model: ModelSpecification):
    """
    The probabilities outputted by model.act() should add to 1.
    """
    observation = [torch.Tensor([1, 2, 3]), torch.Tensor([1, 2, 3, 4, 5, 6])]
    _, probs, _ = model.act(observation)
    assert pytest.approx(1, probs[0].sum().item())


def test_model_outputs_correct_shape_for_multiple_timestep_batches(
        model: ModelSpecification):
    """
    When passing multiple timesteps through forward() at once, the output
    should return multiple timesteps correctly processed.
    """
    observation = [torch.Tensor([1, 2, 3]), torch.Tensor([1, 2, 3, 4, 5, 6])]
    observation = [torch.stack([x, x]) for x in observation]
    _, output = model(observation)

    assert output[0].shape == (2, 2)
    assert output[1].shape == (2, 10)
