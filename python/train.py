from datetime import datetime
import logging
import os
from pathlib import Path
import sys
import time
import singularity_trainer
import tensorflow as tf


def main():
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
        test_number = 0
        while True:
            trainer.step_batch()
            if time.time() - last_test_time > 600:
                test_number += 1
                logging.info("######## Testing ########")
                elo = trainer.evaluate()
                logging.info("Result: %f", elo)
                with writer.as_default():
                    tf.summary.scalar("elo", elo, step=test_number)
                logging.info("######## Tested  ########")
                last_test_time = time.time()
    except KeyboardInterrupt:
        return


if __name__ == '__main__':
    main()
