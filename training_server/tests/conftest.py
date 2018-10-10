"""
Testing utilities.
"""
# pylint: disable=wrong-import-position
import os
os.environ['OMP_NUM_THREADS'] = '1'
import pytest
import torch
import numpy as np


@pytest.fixture(scope="session", autouse=True)
def setup():
    """
    Initialises PyTorch, Numpy, and Cuda.
    """
    if torch.cuda.is_available():
        torch.cuda.init()
    torch.manual_seed(1)
    np.random.seed(1)


def pytest_addoption(parser):
    """
    Adds GPU option for testing GPU function.
    """
    parser.addoption('--gpu', action='store_true', dest="gpu",
                     default=False, help="enable gpu decorated tests")


def pytest_configure(config):
    """
    Adds functionality to GPU option.
    """
    if not config.option.gpu:
        if not config.option.markexpr:
            setattr(config.option, 'markexpr', 'not gpu')
        else:
            setattr(config.option, 'markexpr', (config.option.markexpr
                                                + ' and not gpu'))
