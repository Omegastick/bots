import os
import logging
from pathlib import Path
import sys
import singularity_trainer
import time

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

    try:
        last_test_time = time.time()
        test_number = 1
        while True:
            trainer.step()
            if time.time() - last_test_time > 600:
                logging.info("######## Testing ########")
                winrate = trainer.evaluate(1000)
                logging.info("Result: %f", winrate)
                logging.info("######## Tested  ########")
                last_test_time = time.time()
    except KeyboardInterrupt:
        return


if __name__ == '__main__':
    main()
