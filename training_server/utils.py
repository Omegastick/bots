"""
Training utilities.
"""

import numpy as np


class RunningMeanStd(object):
    """
    https://github.com/openai/baselines/blob/master/baselines/common/running_mean_std.py
    """

    def __init__(self, epsilon=1e-4, shape=()):
        self.mean = np.zeros(shape, 'float64')
        self.var = np.ones(shape, 'float64')
        self.count = epsilon

    def update(self, x):
        """
        Update the running average.
        """
        x = np.array(x, dtype=np.float64)
        if x.size > 1:
            batch_mean = np.mean(x, axis=0)
            batch_var = np.var(x, axis=0)
        else:
            batch_mean = x
            batch_var = 0
        batch_count = x.shape[0]
        self.update_from_moments(batch_mean, batch_var, batch_count)

    def update_from_moments(self, batch_mean, batch_var, batch_count):
        """
        Update the running average from a batch.
        """
        self.mean, self.var, self.count = update_mean_var_count_from_moments(
            self.mean,
            self.var,
            self.count,
            batch_mean,
            batch_var,
            batch_count
        )


def update_mean_var_count_from_moments(
        mean,
        var,
        count,
        batch_mean,
        batch_var,
        batch_count):
    """
    https://github.com/openai/baselines/blob/master/baselines/common/running_mean_std.py
    """
    delta = batch_mean - mean
    tot_count = count + batch_count

    new_mean = mean + delta * batch_count / tot_count
    m_a = var * count
    m_b = batch_var * batch_count
    m_2 = m_a + m_b + np.square(delta) * count * \
        batch_count / (count + batch_count)
    new_var = m_2 / (count + batch_count)
    new_count = batch_count + count

    return new_mean, new_var, new_count