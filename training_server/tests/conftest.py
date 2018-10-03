"""
Testing utilities.
"""
import pytest
import torch
import numpy as np


@pytest.fixture(scope="session", autouse=True)
def setup():
    """
    Initialises PyTorch, Numpy, and Cuda.
    """
    torch.cuda.init()
    torch.manual_seed(1)
    np.random.seed(1)

def pytest_addoption(parser):
    parser.addoption('--gpu', action='store_true', dest="gpu",
                 default=False, help="enable gpu decorated tests")

def pytest_configure(config):
    if not config.option.gpu:
        setattr(config.option, 'markexpr', 'not gpu')