"""
Contains a class that trains an agent.
"""
import logging
import numpy as np
import gym
from baselines.common.cmd_util import make_vec_env
from baselines.common.vec_env import VecEnvWrapper
from training_server.train import HyperParams
from training_server.model import ModelSpecification

from gym_client.client import Client
from gym_client.requests import (BeginTrainingSessionRequest, GetActionRequest,
                                 GiveRewardRequest)


RUNNING_REWARD_HORIZON = 10


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
        elif env_type == 'atari':
            self.env = VecPytorchImageFormat(self.env)
            model_specification = ModelSpecification(
                inputs=[list(self.env.observation_space.shape)],
                outputs=[self.env.action_space.n],
                feature_extractors=["cnn"])
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
        running_reward = 0

        while current_frame < max_frames:
            # Get actions
            actions = []
            for i, observation in enumerate(observations):
                get_action_request = GetActionRequest(
                    [observation.tolist()], i, 0)
                actions.append(
                    self.client.send_request(
                        get_action_request)["result"]["actions"][0])

            # Step environments
            observations, rewards, dones, _ = self.env.step(actions)

            # Give rewards
            for i, reward in enumerate(rewards):
                give_reward_request = GiveRewardRequest(
                    float(reward), bool(dones[i]), i, 0)
                self.client.send_request(give_reward_request)

            # Increment frame counter
            current_frame += self.env.num_envs

            # Update episode rewards
            for i, _ in enumerate(episode_rewards):
                episode_rewards[i] += rewards[i]
                if dones[i]:
                    running_reward -= running_reward / RUNNING_REWARD_HORIZON
                    running_reward += (episode_rewards[i]
                                       / RUNNING_REWARD_HORIZON)
                    logging.info("Frame: %i - Reward: %f - Average: %f",
                                 current_frame,
                                 episode_rewards[i],
                                 running_reward)
                    episode_rewards[i] = 0


class VecPytorchImageFormat(VecEnvWrapper):
    def __init__(self, venv):
        super().__init__(venv)
        old_shape = self.observation_space.shape
        self.observation_space = gym.spaces.Box(
            low=0.0,
            high=1.0,
            shape=(old_shape[-1], old_shape[0], old_shape[1]),
            dtype=np.uint8)

    def step_wait(self):
        obs, rews, news, infos = self.venv.step_wait()
        obs = obs.swapaxes(-1, -3)
        return obs, rews, news, infos

    def reset(self):
        obs = self.venv.reset()
        obs = obs.swapaxes(-1, -3)
        return obs
