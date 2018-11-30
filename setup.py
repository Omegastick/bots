#!/usr/bin/env python

from distutils.core import setup

setup(name='singularity_trainer',
      version='0.1',
      description='Training server for Singularity Trainer',
      author='Isaac Poulton',
      author_email='omegastick@hotmail.co.uk',
      packages=['training_server', 'gym_client']
      )
