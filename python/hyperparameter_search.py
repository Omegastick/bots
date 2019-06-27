from hyperopt import hp
import numpy as np
import pickle
import json
import os
import random
import ray
import ray.tune as tune
from ray.tune.suggest.hyperopt import HyperOptSearch
from ray.tune.logger import Logger, JsonLogger, CSVLogger
from ray.tune.result import (NODE_IP, TRAINING_ITERATION, TIME_TOTAL_S,
                             TIMESTEPS_TOTAL)
import singularity_trainer as st
import tensorflow as tf
import time


def to_tf_values(result, path):
    values = []
    for attr, value in result.items():
        if value is not None:
            type_list = [int, float, np.float32, np.float64, np.int32]
            if type(value) in type_list:
                values.append(
                    ("/".join(path + [attr]), value))
            elif type(value) is dict:
                values.extend(to_tf_values(value, path + [attr]))
    return values


class TFEagerLogger(Logger):
    def _init(self):
        self._file_writer = tf.summary.create_file_writer(self.logdir)
        self._file_writer.set_as_default()

    def on_result(self, result):
        tmp = result.copy()
        for k in [
                "config", "pid", "timestamp", TIME_TOTAL_S, TRAINING_ITERATION
        ]:
            if k in tmp:
                del tmp[k]  # not useful to tf log these
        values = to_tf_values(tmp, ["ray", "tune"])
        iteration_value = to_tf_values({
            "training_iteration": result[TRAINING_ITERATION]
        }, ["ray", "tune"])
        t = result.get(TIMESTEPS_TOTAL) or result[TRAINING_ITERATION]
        with self._file_writer.as_default():
            for value in values + iteration_value:
                tf.summary.scalar(value[0], value[1], step=t)
        self._file_writer.flush()

    def flush(self):
        self._file_writer.flush()

    def close(self):
        self._file_writer.close()


class HyperParameterSearch(tune.Trainable):

    def _setup(self, config):
        self._timesteps_total = 0
        with open(config["base_program"], 'r') as file:
            self.program = json.load(file)
        self.program["hyper_parameters"] = config
        self.trainer = None

    def _train(self):
        self.trainer = st.make_trainer(json.dumps(self.program, indent=0))
        for _ in range(10000):
            self.trainer.step()
        return {"winrate": self.trainer.evaluate(500)}

    def _save(self, directory):
        return self.trainer.save_model()

    def _restore(self, checkpoint_path):
        self.program["checkpoint"] = checkpoint_path
        self.trainer = st.make_trainer(json.dumps(program, indent=0))


def main():
    ray.init()
    hyperband = tune.schedulers.AsyncHyperBandScheduler(
        time_attr="training_iteration",
        metric="winrate",
        mode="max",
        max_t=20)
    experiment = tune.Experiment(
        name="a2c_hyperparameter_search",
        run=HyperParameterSearch,
        stop={},
        num_samples=32,
        loggers=[JsonLogger, CSVLogger, TFEagerLogger],
        resources_per_trial={"cpu": 4})
    algo = HyperOptSearch(
        {
            "base_program": os.getcwd() + "/../programs/asd.json",
            "actor_loss_coef": hp.uniform("actor_loss_coef", 0.25, 1),
            "algorithm": 0,
            "batch_size": hp.qloguniform("batch_size", 1, 5, 1),
            "clip_param": hp.uniform("clip_param", 0.075, 0.3),
            "discount_factor": hp.loguniform("discount_factor", -1, 0),
            "entropy_coef": hp.loguniform("entropy_coef", -15, -4),
            "learning_rate": 0.0001,
            "num_env": 4,
            "num_epoch": 3,
            "num_minibatch": 32,
            "value_loss_coef": hp.uniform("value_loss_coef", 0.25, 1)
        },
        max_concurrent=999,
        metric="winrate",
        mode="max")
    tune.run(experiment,
             search_alg=algo,
             scheduler=hyperband,
             resume="prompt")


if __name__ == '__main__':
    main()
