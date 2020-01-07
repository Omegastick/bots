"""
Conftest for AI: Artificial Insentience cloud.
"""

import os

from google.cloud import container_v1, firestore
from kubernetes.client import V1DeleteOptions
import pytest

from main import kubernetes_api

BASE_URL = os.environ['ST_CLOUD_BASE_URL']


def pytest_addoption(parser):
    """
    Add integration test option.
    """
    parser.addoption("--integration",
                     action="store_true",
                     default=False,
                     help="run integration tests")


def pytest_configure(config):
    """
    Add line to config inivalue.
    """
    config.addinivalue_line(
        "markers", "integration: mark test as integration to run")


def pytest_collection_modifyitems(config, items):
    """
    Skip integration tests by default.
    """
    if config.getoption("--integration"):
        # --integration given in cli: do not skip integration tests
        return
    skip_integration = pytest.mark.skip(
        reason="need --integration option to run")
    for item in items:
        if "integration" in item.keywords:
            item.add_marker(skip_integration)


@pytest.fixture
def db():
    """
    Provides a real cloud database, with all test users removed afterwards.
    """
    database = firestore.Client()
    yield database

    users = database.collection('users')

    batch = database.batch()

    for user in (users
                 .where('username', '>=', '__')
                 .where('username', '<', '_`')).stream():
        batch.delete(users.document(user.id))

    batch.commit()

    gke_client = container_v1.ClusterManagerClient()
    cluster = gke_client.get_cluster(
        'st-dev-252104', 'asia-northeast1-b', 'st-dev')
    k8s = kubernetes_api(cluster)

    gameservers = k8s.get_namespaced_custom_object('agones.dev',
                                                   'v1',
                                                   'default',
                                                   'gameservers',
                                                   '')['items']

    for gameserver in gameservers:
        if gameserver['status']['state'] == 'Ready':
            continue
        k8s.delete_namespaced_custom_object('agones.dev',
                                            'v1',
                                            'default',
                                            'gameservers',
                                            gameserver['metadata']['name'],
                                            V1DeleteOptions())
