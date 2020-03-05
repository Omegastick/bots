#!/bin/bash

set -e

function cleanup {
    cd ..
    rm -r packaging
}
trap cleanup EXIT

VERSION=0.0.0
FILENAME="artificial-insentience-${VERSION}-linux.tar.gz"


# Set up temp directory
mkdir -p packaging
cd packaging || exit

# Copy executable
cp ../build/artificial_insentience .

# Copy assets
cp -r ../assets .

# Copy dependencies
wget https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-1.4.0%2Bcpu.zip 
unzip libtorch-shared-with-deps-1.4.0%2Bcpu.zip  libtorch/lib/*
mv libtorch/lib/*.so* .
rm -rf libtorch/
rm libtorch-shared-with-deps-1.4.0%2Bcpu.zip 

# Create archive
tar -czvf "${FILENAME}" ./*
mv "${FILENAME}" ..
