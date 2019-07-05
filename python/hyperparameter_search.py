import json
import os
import sys
import time
from hyperopt import hp
import numpy as np
import ray
import ray.tune as tune
from ray.tune.suggest.hyperopt import HyperOptSearch
from ray.tune.logger import Logger, JsonLogger, CSVLogger
from ray.tune.result import TRAINING_ITERATION, TIME_TOTAL_S, TIMESTEPS_TOTAL
import singularity_trainer as st
import tensorflow as tf


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
        self.trainer = st.make_trainer(json.dumps(self.program, indent=0))

    def _train(self):
        for _ in range(5000):
            self.trainer.step()
        return {"elo": self.trainer.evaluate()}

    def _save(self, _):
        return {"path": self.trainer.save_model("")}

    def _restore(self, checkpoint):
        self.program["checkpoint"] = checkpoint["path"]
        checkpoint_dir = os.path.dirname(program["checkpoint"])
        self.program["opponent_pool"] = []
        for file in os.listdir(checkpoint_dir):
            if file.endswith(".meta"):
                self.program["opponent_pool"].append(file)
        self.trainer = st.make_trainer(json.dumps(self.program, indent=0))


def main():
    ray.init(local_mode=False)
    hyperband = tune.schedulers.HyperBandScheduler(
        time_attr="training_iteration",
        metric="elo",
        mode="max",
        max_t=75)
    experiment = tune.Experiment(
        name="ppo_hyperparameter_search_4",
        run=HyperParameterSearch,
        stop={},
        num_samples=128,
        loggers=[JsonLogger, CSVLogger, TFEagerLogger],
        resources_per_trial={"cpu": 2})
    algo = HyperOptSearch(
        {
            "base_program": os.path.join(os.getcwd(), sys.argv[1]),
            "actor_loss_coef": hp.uniform("actor_loss_coef", 0.25, 1),
            "algorithm": 1,
            "batch_size": hp.qloguniform("batch_size", 2.5, 8, 1),
            "clip_param": hp.uniform("clip_param", 0.05, 0.3),
            "discount_factor": hp.loguniform("discount_factor", -1, 0),
            "entropy_coef": hp.loguniform("entropy_coef", -15, -4),
            "learning_rate": hp.choice("learning_rate", [0.001, 0.0001,
                                                         0.00001]),
            "num_env": 8,
            "num_epoch": hp.choice("num_epoch", range(2, 9)),
            "num_minibatch": 8,
            "value_loss_coef": hp.uniform("value_loss_coef", 0.25, 1)
        },
        max_concurrent=8,
        metric="elo",
        mode="max",
        points_to_evaluate=[{
            "actor_loss_coef": 0.9858908897512276,
            "algorithm": 1,
            "base_program": os.path.join(os.getcwd(), sys.argv[1]),
            "batch_size": 203.0,
            "clip_param": 0.2966414935576334,
            "discount_factor": 0.850793792894193,
            "entropy_coef": 0.011559455852253061,
            "learning_rate": 1,
            "num_env": 8,
            "num_epoch": 5,
            "num_minibatch": 8,
            "value_loss_coef": 0.5323687551335846
        },
            {
            "actor_loss_coef": 0.8185698586238065,
            "algorithm": 1,
            "base_program": os.path.join(os.getcwd(), sys.argv[1]),
            "batch_size": 27.0,
            "clip_param": 0.22135255042157442,
            "discount_factor": 0.4871560553665869,
            "entropy_coef": 1.2129990452636745e-06,
            "learning_rate": 1,
            "num_env": 8,
            "num_epoch": 3,
            "num_minibatch": 8,
            "value_loss_coef": 0.3927127459992794
        }])
    tune.run(
        experiment,
        search_alg=algo,
        scheduler=hyperband,
        resume="prompt")


if __name__ == '__main__':
    main()
