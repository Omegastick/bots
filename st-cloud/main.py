"""
Serverless functions for the Singularity Trainer matchmaking server.
"""
# pylint: disable=invalid-name

import base64
import json
import secrets
from tempfile import NamedTemporaryFile
from typing import List, Tuple

from flask import abort, Request
from google.cloud import container_v1, firestore
import googleapiclient.discovery
import kubernetes

# Init database
db = firestore.Client()


def get_k8s_token(*scopes: List[str]) -> str:
    """
    Get a Google access token with the given scopes.
    """
    # pylint: disable=protected-access
    credentials = googleapiclient._auth.default_credentials()
    scopes = [f'https://www.googleapis.com/auth/{s}' for s in scopes]
    scoped = googleapiclient._auth.with_scopes(credentials, scopes)
    googleapiclient._auth.refresh_credentials(scoped)
    return scoped.token


def kubernetes_api(cluster) -> kubernetes.client.CustomObjectsApi:
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


def login(request: Request) -> str:
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


def find_game(request: Request) -> str:
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
        abort(401, "Invalid authorization")
    if len(matching_users) > 1:
        raise RuntimeError("Duplicate tokens in database")

    user = matching_users[0]
    user_dict = user.to_dict()
    if user_dict.get('status', None) == 'in_game':
        return json.dumps({
            'status': 'in_game',
            'gameserver': user_dict['gameserver']
        })

    waiting_users_query = users.where('status', '==', 'waiting_for_game')
    waiting_users = [x for x in waiting_users_query.stream()
                     if x.id != user.id]

    if not waiting_users:
        return wait_for_game(users, user)

    player_1_dict = user.to_dict()
    player_2_dict = waiting_users[0].to_dict()
    gameserver = allocate_gameserver(
        [player_1_dict['username'], player_2_dict['username']],
        [player_1_dict['token'], player_2_dict['token']])
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


def finish_game(request: Request) -> str:
    """
    Given two player and a match result, update their Elos in the database.

    Input Json in the form:
    {
        "players": ["username_1", "username_2"],
        "result": 0
    }

    -1 is a draw.
    0 is player 1 win.
    1 is player 2 win.
    """
    # Check authorization
    auth_header = request.headers.get("Authorization")
    if not auth_header:
        abort(401, "No authorization header")
    token = auth_header.split(' ')[1]

    secret = db.collection('secrets').document('server').get().get('value')
    if token != secret:
        abort(401, "Invalid authorization")

    # Check body
    request_json = request.json
    if 'players' not in request_json or 'result' not in request_json:
        abort(400, "No username provided")
    if (not isinstance(request_json['players'], list)
            or len(request_json['players']) != 2):
        abort(400, "'players' must be a list of two usernames")

    # Get users
    users = db.collection('users')

    user_1_query = users.where('username', '==', request_json['players'][0])
    user_1 = list(user_1_query.stream())
    if not user_1:
        raise RuntimeError(f"No user named {request_json['players'][0]}")
    user_1 = user_1[0]

    user_2_query = users.where('username', '==', request_json['players'][1])
    user_2 = list(user_2_query.stream())
    if not user_2:
        raise RuntimeError(f"No user named {request_json['players'][1]}")
    user_2 = user_2[0]

    # Calculate Elos
    user_1_elo, user_2_elo = calculate_elos(user_1.to_dict()['elo'],
                                            user_2.to_dict()['elo'],
                                            10,
                                            request_json['result'])

    # Update Elos in database
    batch = db.batch()
    batch.update(users.document(user_1.id), {'elo': user_1_elo,
                                             'status': 'idle'})
    batch.update(users.document(user_2.id), {'elo': user_2_elo,
                                             'status': 'idle'})
    batch.commit()

    return json.dumps({'success': True})


def wait_for_game(users: firestore.CollectionReference,
                  user: firestore.DocumentSnapshot) -> str:
    """
    Mark the user as waiting for a game, and return the appropriate Json.
    """
    users.document(user.id).update({'status': 'waiting_for_game'})
    return json.dumps({'status': 'waiting_for_game'})


def allocate_gameserver(player_usernames: List[str],
                        player_tokens: List[str]) -> str:
    """
    Allocate a gameserver and return its info.
    """
    deployment = {"api_version": "allocation.agones.dev/v1",
                  "kind": "GameServerAllocation",
                  "spec": {
                      "required": {
                          "matchLabels": {
                              "agones.dev/fleet": "singularity-trainer"
                          }
                      },
                      "metadata": {
                          "annotations": {
                              "player_1_username": player_usernames[0],
                              "player_2_username": player_usernames[1],
                              "player_1_token": player_tokens[0],
                              "player_2_token": player_tokens[1]
                          }
                      }
                  }}

    def send_allocate_request(deployment):
        return k8s.create_namespaced_custom_object('allocation.agones.dev',
                                                   'v1',
                                                   'default',
                                                   'gameserverallocations',
                                                   deployment)
    response = None
    try:
        response = send_allocate_request(deployment)
    except kubernetes.client.rest.ApiException:
        token = get_k8s_token('cloud-platform')
        k8s.api_client.configuration.api_key['authorization'] = token
        response = send_allocate_request(deployment)

    return response


def expected_win_chance(a_rating: float, b_rating: float) -> float:
    """
    Calculate A's expected chance of winning [0-1].
    """
    return 1. / (1. + (10 ** ((b_rating - a_rating) / 400.)))


def calculate_elos(
        a_rating: float,
        b_rating: float,
        k: float,
        result: int) -> Tuple[float, float]:
    """
    Calculate new elos based on ratings, an Elo constant and the game result.
    """
    a_win_chance = expected_win_chance(a_rating, b_rating)
    b_win_chance = 1. - a_win_chance

    if result == 0:
        a_rating = a_rating + k * (1 - a_win_chance)
        b_rating = b_rating + k * (0 - b_win_chance)
    elif result == 1:
        a_rating = a_rating + k * (0 - a_win_chance)
        b_rating = b_rating + k * (1 - b_win_chance)
    else:
        a_rating = a_rating + k * (0.5 - a_win_chance)
        b_rating = b_rating + k * (0.5 - b_win_chance)

    return (a_rating, b_rating)
