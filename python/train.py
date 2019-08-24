"""
Train an agent using Singularity Trainer.
"""
from datetime import datetime
import logging
import os
import sys
import time
import tensorflow as tf

import singularity_trainer


def main():
    """
    Run the training.
    """
    # If anything is logged during imports, it messes up our logging so we
    # reset the logging module here
    root_logger = logging.getLogger()
    if root_logger.handlers:
        for handler in root_logger.handlers:
            root_logger.removeHandler(handler)
    logging.basicConfig(level=logging.DEBUG,
                        format=('%(asctime)s %(funcName)s '
                                '[%(levelname)s]: %(message)s'),
                        datefmt='%Y%m%d %H:%M:%S')

    with open(sys.argv[1], 'r') as file:
        trainer = singularity_trainer.make_trainer(file.read())

    log_dir = os.path.join(
        "runs", datetime.now().strftime("%Y%m%d-%H%M%S"))
    writer = tf.summary.create_file_writer(log_dir)

    try:
        last_test_time = time.time()
        batch_number = 0
        while True:
            batch_number += 1
            training_stats = trainer.step_batch()
            with writer.as_default():
                for stat in training_stats:
                    tf.summary.scalar(stat[0], stat[1], step=batch_number)
            if time.time() - last_test_time > 60:
                last_test_time = time.time()
                logging.info("######## Testing ########")
                elo = trainer.evaluate()
                logging.info("Result: %f", elo)
                with writer.as_default():
                    tf.summary.scalar("Elo", elo, step=batch_number)
                logging.info("######## Tested  ########")
    except KeyboardInterrupt:
        return


if __name__ == '__main__':
    main()
