"""
Contains a class that trains an agent.
"""

import logging
from baselines.common.cmd_util import make_vec_env
from training_server.train import HyperParams
from training_server.model import ModelSpecification

from gym_client.client import Client
from gym_client.requests import (BeginTrainingSessionRequest, GetActionRequest,
                                 GiveRewardRequest)


class Trainer:
    """
    Trains an agent on a gym environment.
    """

    def __init__(
            self,
            hyperparams: HyperParams,
            num_environments: int,
            env_name: str,
            env_type: str,
            client: Client):
        self.client = client
        self.env = make_vec_env(env_name, env_type, num_environments, 0)

        if env_type == 'linear':
            model_specification = ModelSpecification(
                inputs=[self.env.observation_space.shape[0]],
                outputs=[self.env.action_space.n],
                feature_extractors=["mlp"])
        else:
            raise NotImplementedError()

        # Begin training session
        begin_training_session_request = BeginTrainingSessionRequest(
            model_specification, hyperparams, num_environments, 0)
        self.client.send_request(begin_training_session_request)

    def train(self, max_frames):
        """
        Trains until max_frames has been reached.
        """
        current_frame = 0

        observations = self.env.reset()

        episode_rewards = [0. for _ in range(self.env.num_envs)]

        while current_frame < max_frames:
            # Get actions
            actions = []
            for i, observation in enumerate(observations):
                get_action_request = GetActionRequest(
                    [list(observation)], i, 0)
                actions.append(
                    self.client.send_request(
                        get_action_request)["result"]["actions"][0])

            # Step environments
            observations, rewards, dones, _ = self.env.step(actions)

            # Give rewards
            for i, reward in enumerate(rewards):
                give_reward_request = GiveRewardRequest(
                    reward, bool(dones[i]), i, 0)
                self.client.send_request(give_reward_request)

            # Increment frame counter
            current_frame += self.env.num_envs

            # Update episode rewards
            for i, _ in enumerate(episode_rewards):
                episode_rewards[i] += rewards[i]
                if dones[i]:
                    logging.info("Frame: %i - Reward: %f",
                                 current_frame, episode_rewards[i])
                    episode_rewards[i] = 0
