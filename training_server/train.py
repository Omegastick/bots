"""
Train
"""
import logging
from typing import NamedTuple, List, Tuple
from collections import deque
import torch
import torch.nn as nn
import numpy as np
from a2c_ppo_acktr.algo import PPO
from a2c_ppo_acktr.storage import RolloutStorage
from gym import spaces

from .model import CustomPolicy, ModelSpecification


class HyperParams(NamedTuple):
    """
    Named tuple for storing hyperparameters.
    """
    learning_rate: float = 0.0001
    batch_size: int = 250
    num_minibatch = 32
    epochs: int = 5
    discount_factor: float = 0.99
    gae: float = 1.
    critic_coef: float = 0.5
    entropy_coef: float = 0.001
    max_grad_norm: float = 0.5
    clip_factor: float = 0.2
    use_gpu: bool = False


class TrainingSession:
    """
    Manages a training session for an agent.
    """

    def __init__(
            self,
            model: ModelSpecification,
            hyperparams: HyperParams,
            contexts: int
    ):
        torch.set_num_threads(1)
        device = torch.device("cuda:0" if hyperparams.use_gpu else "cpu")

        self.model = CustomPolicy(model.inputs, model.outputs,
                                  model.feature_extractors, model.kernel_sizes,
                                  model.kernel_strides, model.recurrent)
        self.model.to(device)

        self.agent = PPO(self.model, hyperparams.clip_factor,
                         hyperparams.epochs, hyperparams.num_minibatch,
                         hyperparams.critic_coef, hyperparams.entropy_coef,
                         hyperparams.learning_rate, 1e-5,
                         hyperparams.max_grad_norm)

        observation_spaces = []
        for input_ in model.inputs:
            if isinstance(input_, int):
                observation_spaces.append(spaces.Box(input_))
            else:
                observation_spaces.append(spaces.Box(shape=input_))
        self.rollouts = RolloutStorage(hyperparams.batch_size, contexts,
                                       spaces.Tuple(observation_spaces),
                                       spaces.MultiBinary(model.outputs), 128)

        self.episode_rewards = deque(maxlen=10)

        # TrainingServer is a state machine
        # It has three states:
        # - wating_for_observation: Waiting to be sent an observation.
        #                           When an observation is received, it will
        #                           return an action and wait for the next
        #                           observation.
        # - waiting_for_reward: Waiting to be sent a reward.
        #                       After sending an action, the agent expects a
        #                       reward.
        #                       Upon receiving a reward, stores it/updates based
        #                       on it and waits for an observation.
        # - start: We are waiting for an observation, but when we receive it we
        #          also haven't received a reward yet so can't do the normal
        #          processing.
        self.state = "start"

    def get_action(
            self,
            inputs: List[torch.Tensor],
            context: int) -> Tuple[List[int], torch.Tensor]:
        """
        Given an observation, get an action and the value of the observation
        from one of the models being trained.
        """
        raise NotImplementedError()

    def give_reward(self, reward: float, context: int, done: bool = False):
        """
        Assign a reward to the last action performed.
        """
        raise NotImplementedError()

    def save_model(self, path: str):
        """
        Saves the current model to disk.
        """
        torch.save(self.model.state_dict(), path)
