"""
Tests for model.py
"""
#pylint: disable=W0621
import pytest
import torch

from training_server.model import CustomPolicy, CustomBase


@pytest.fixture
def model():
    """
    Returns a new, randomly parameterised model.
    Inputs: 3, 6
    Outputs: 5
    Features: mlp, mlp
    """
    return CustomPolicy([3, 6], 5, ['mlp', 'mlp'])


def test_base_output_dimensions():
    """
    The outputted raw probabilities should match dimensions specified in the
    constructor.
    """
    base = CustomBase([3, 6], ['mlp', 'mlp'])
    observation = [torch.Tensor([1, 2, 3]), torch.Tensor([1, 2, 3, 4, 5, 6])]
    _, features, _ = base(observation)

    expected = [1, 128]
    dimensions = list(features.shape)

    assert dimensions == expected


def test_base_outputs_correct_shape_for_multiple_timestep_batches():
    """
    When passing multiple timesteps through forward() at once, the output
    should return multiple timesteps correctly processed.
    """
    base = CustomBase([3, 6], ['mlp', 'mlp'])
    observation = [torch.Tensor([1, 2, 3]), torch.Tensor([1, 2, 3, 4, 5, 6])]
    observation = [torch.stack([x, x]) for x in observation]
    _, output, _ = base(observation)

    assert list(output.shape) == [2, 128]


def test_cnn_base_output_shape():
    """
    When using a model with CNN features, it should output the correct shape.
    """
    base = CustomBase(
        inputs=[3, [1, 5, 6]],
        feature_extractors=['mlp', 'cnn'],
        kernel_sizes=[3, 2, 1],
        kernel_strides=[1, 1, 1]
    )
    observation = [torch.Tensor([[1, 2, 3]]),
                   torch.Tensor([[[[1, 2, 3, 4, 5, 6],
                                   [1, 2, 3, 4, 5, 6],
                                   [1, 2, 3, 4, 5, 6],
                                   [1, 2, 3, 4, 5, 6],
                                   [1, 2, 3, 4, 5, 6]]]])]
    _, output, _ = base(observation)

    assert list(output.shape) == [1, 128]
