#!/bin/bash

set -e

sudo docker build . -t st-server
sudo docker tag st-server gcr.io/$GPID/st-server
sudo docker push gcr.io/$GPID/st-server