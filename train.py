from typing import NamedTuple, List
import torch

from model import Model


class ModelSpecification(NamedTuple):
    """
    Named tuple for specifying a model to be created later.
    """
    inputs: List[int]
    outputs: List[int]
    feature_extractors: List[str]


class HyperParams(NamedTuple):
    """
    Named tuple for storing hyperparameters.
    """
    learning_rate: float = 0.0001
    batch_size: int = 5


class TrainingSession(object):
    """
    Manages a training session for an agent.
    """

    def __init__(
            self,
            model: ModelSpecification,
            hyperparams: HyperParams,
            contexts: int,
            auto_train: bool = True):
        self.hidden_states = torch.zeros(contexts, 128)
        self.hyperparams = hyperparams
        self.auto_train = auto_train

        self.rewards = [[] for _ in range(contexts)]
        self.log_probs = [[] for _ in range(contexts)]
        self.values = [[] for _ in range(contexts)]

        self.shared_model = Model(model.inputs, model.outputs,
                                  model.feature_extractors)

        self.models = [
            Model(model.inputs, model.outputs, model.feature_extractors)
            for _ in range(contexts)]

    def get_action(self, inputs: List[torch.Tensor], context: int):
        value, probs, log_probs = self.models[context].act(inputs)

        actions = [prob.argmax() for prob in probs]

        self.log_probs[context].append(torch.cat(*log_probs))

        if self.auto_train:
            if (min([len(x) for x in self.rewards])
                    >= self.hyperparams.batch_size):
                self.train()

        return actions, value

    def give_reward(self, reward: float, context: int):
        if len(self.rewards[context]) != len(self.log_probs[context]) - 1:
            raise Exception(
                f"Rewards and actions out of sync.\n"
                f"{len(self.rewards[context])} rewards and "
                f"{len(self.log_probs[context])} actions have been recorded.")
        else:
            self.rewards[context].append(reward)

    def train(self):
        raise NotImplementedError()
