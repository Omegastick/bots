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
from gym_client.requests import (BeginTrainingSessionRequest,
                                 GetActionsRequest, GiveRewardsRequest,
                                 EndSessionRequest, CloseConnectionRequest)


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
                inputs=self.env.observation_space.shape[0],
                outputs=self.env.action_space.n,
                recurrent=True,
                normalize_observations=True)
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
            get_actions_request = GetActionsRequest(observations, 0)
            actions = self.client.send_request(
                get_actions_request)["result"]["actions"]
            actions = [np.argmax(action) for action in actions]

            # Step environments
            observations, rewards, dones, _ = self.env.step(actions)

            # Give rewards
            give_reward_request = GiveRewardsRequest(
                [float(reward) for reward in rewards],
                [bool(done) for done in dones],
                0)
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
                    logging.info("Frame: %i - Reward: %.2f - Average: %.2f",
                                 current_frame,
                                 episode_rewards[i],
                                 running_reward)
                    episode_rewards[i] = 0

    def end_session(self):
        """
        Tells the server that training has finished.
        """
        end_session_request = EndSessionRequest(0)
        self.client.send_request(end_session_request)
        close_connection_request = CloseConnectionRequest()
        self.client.send_request(close_connection_request)


class VecPytorchImageFormat(VecEnvWrapper):
    """
    Environment wrapper that reshapes the observations from
    [env, width, height, channel] to [env, channel, width, height] (ie. PyTorch
    format).
    """

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
