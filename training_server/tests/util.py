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
