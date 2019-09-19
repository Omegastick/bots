"""
Serverless functions for the Singularity Trainer matchmaking server.
"""
# pylint: disable=invalid-name

import base64
import json
import secrets
from tempfile import NamedTemporaryFile

from flask import abort
from google.cloud import container_v1, firestore
import googleapiclient.discovery
import kubernetes

# Init database
db = firestore.Client()


def get_k8s_token(*scopes):
    """
    Get a Google access token with the given scopes.
    """
    # pylint: disable=protected-access
    credentials = googleapiclient._auth.default_credentials()
    scopes = [f'https://www.googleapis.com/auth/{s}' for s in scopes]
    scoped = googleapiclient._auth.with_scopes(credentials, scopes)
    googleapiclient._auth.refresh_credentials(scoped)
    return scoped.token


def kubernetes_api(cluster):
    """
    Get the K8s custom objects API for a given cluster.
    """
    config = kubernetes.client.Configuration()
    config.host = f'https://{cluster.endpoint}'

    config.api_key_prefix['authorization'] = 'Bearer'
    config.api_key['authorization'] = get_k8s_token('cloud-platform')

    with NamedTemporaryFile(delete=False) as cert:
        cert.write(base64.decodebytes(
            cluster.master_auth.cluster_ca_certificate.encode()))
        config.ssl_ca_cert = cert.name

    client = kubernetes.client.ApiClient(configuration=config)
    api = kubernetes.client.CustomObjectsApi(client)

    return api


# Init Kubernetes client
gke_client = container_v1.ClusterManagerClient()
k8s = kubernetes_api(gke_client.get_cluster('st-dev-252104',
                                            'asia-northeast1-b',
                                            'st-dev'))


def login(request):
    """
    Log a user into the matchmaking system, returning a session token.
    If the user does not exist, creates one.
    Raises an exception if multiple users with the same username exist.
    """
    request_json = request.json
    if 'username' not in request_json:
        abort(400, "No username provided")
    username = request.json['username']

    users = db.collection('users')

    # Get all users matching the requested username
    matching_users_query = users.where('username', '==', username)
    matching_users = list(matching_users_query.stream())
    user: firestore.DocumentReference = None

    if len(matching_users) == 1:
        # User exists
        user = users.document(matching_users[0].id)
    elif len(matching_users) > 1:
        # Multiple users with the same username exist
        # This shouldn't happen
        matching_users_dicts = [x.to_dict() for x in matching_users]
        raise RuntimeError(f"{len(matching_users)} matching users in database:"
                           f" {matching_users_dicts}")
    else:
        # Create a new user
        user = users.add({'username': username, 'elo': 0})[1]

    token = secrets.token_urlsafe()
    user.update({'token': token,
                 'token_set_time': firestore.SERVER_TIMESTAMP})

    return json.dumps({'token': token})


def find_game(request):
    """
    Find a game for a user.
    If no other players are waiting, places the player into the waiting queue.
    """
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        abort(401, "No authorization header")
    token = auth_header.split(' ')[1]

    users = db.collection('users')

    matching_users_query = users.where('token', '==', token)
    matching_users = list(matching_users_query.stream())

    if not matching_users:
        abort(401, "Bad authorization token")
    if len(matching_users) > 1:
        raise RuntimeError("Duplicate tokens in database")

    user = matching_users[0]

    waiting_users_query = users.where('status', '==', 'waiting_for_game')
    waiting_users = [x for x in waiting_users_query.stream()
                     if x.id != user.id]

    if not waiting_users:
        return wait_for_game(users, user)

    gameserver = allocate_gameserver()
    if gameserver['status']['state'] != 'Allocated':
        return wait_for_game(users, user)

    batch = db.batch()
    gameserver_url = (f"tcp://{gameserver['status']['address']}:"
                      f"{gameserver['status']['ports'][0]['port']}")
    update_data = {
        'status': 'in_game',
        'gameserver': gameserver_url
    }
    batch.update(users.document(user.id), update_data)
    batch.update(users.document(waiting_users[0].id), update_data)
    batch.commit()

    return json.dumps(update_data)


def wait_for_game(users, user):
    """
    Mark the user as waiting for a game, and return the appropriate Json.
    """
    users.document(user.id).update({'status': 'waiting_for_game'})
    return json.dumps({'status': 'waiting_for_game'})


def allocate_gameserver():
    """
    Allocate a gameserver and return its info.
    """
    depoloyment = {"api_version": "allocation.agones.dev/v1",
                   "kind": "GameServerAllocation",
                   "spec": {
                       "required": {
                           "matchLabels": {
                               "agones.dev/fleet": "singularity-trainer"
                           }
                       }
                   }}

    response = k8s.create_namespaced_custom_object('allocation.agones.dev',
                                                   'v1',
                                                   'default',
                                                   'gameserverallocations',
                                                   depoloyment)

    return response
