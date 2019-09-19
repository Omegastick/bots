"""
Tests for the Singularity Trainer cloud adjust_elos function.
"""

# pylint: disable=no-member

import json
from unittest.mock import patch, MagicMock, Mock

import pytest
import requests

from conftest import BASE_URL
import main

# @patch('main.db', MagicMock())
# def test_adjust_elos_thr
