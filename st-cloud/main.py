"""
Serverless functions for the Singularity Trainer matchmaking server.
"""

import base64
import os
import json
import secrets

from google.cloud import firestore, kms
import requests

# Init database
db = firestore.Client()

# Get Kubernetes token
kms_client = kms.KeyManagementServiceClient()  # pylint: disable=invalid-name
AGONES_TOKEN = kms_client.decrypt(
    os.environ.get('AGONES_TOKEN_RESOURCE_NAME'),
    base64.b64decode(os.environ.get('AGONES_TOKEN'))
).plaintext.decode("ascii")

# Kubernetes base URL
K8S_BASE_URL = "https://35.200.112.204"


def login(request):
    """
    Log a user into the matchmaking system, returning a session token.
    If the user does not exist, creates one.
    Raises an exception if multiple users with the same username exist.
    """
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
    request_json = request.json
    if 'token' not in request_json:
        raise RuntimeError("No token provided")
    token = request.json['token']

    users = db.collection('users')

    matching_users_query = users.where('token', '==', token)
    matching_users = list(matching_users_query.stream())

    if not matching_users:
        raise RuntimeError("Invalid token")
    if len(matching_users) > 1:
        raise RuntimeError("Duplicate tokens in database, request new token")

    user = matching_users[0]

    waiting_users_query = users.where('status', '==', 'waiting_for_game')
    waiting_users = [x for x in waiting_users_query.stream()
                     if x.id != user.id]

    if not waiting_users:
        users.document(user.id).update({'status': 'waiting_for_game'})
        return json.dumps({'status': 'waiting_for_game'})

    batch = db.batch()
    batch.update(users.document(user.id), {'status': 'in_game'})
    batch.update(users.document(waiting_users[0].id), {'status': 'in_game'})
    batch.commit()

    return json.dumps({'status': 'in_game'})


def allocate_gameserver():
    """
    Allocate a gameserver and return its info.
    """
    response = requests.post(
        (K8S_BASE_URL + '/apis/allocation.agones.dev/v1/namespaces/default/'
         + 'gameserverallocations'),
        verify=False,
        json={"api_version": "allocation.agones.dev/v1",
              "kind": "GameServerAllocation",
              "spec": {
                      "required": {
                          "matchLabels": {
                              "agones.dev/fleet": "singularity-trainer"
                          }
                      }
              }},
        headers={'Authorization': f'Bearer {AGONES_TOKEN}'})
    return response.json()


if __name__ == '__main__':
    allocate_gameserver()
