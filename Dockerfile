FROM gcc:9 as builder

# Install PyTorch
RUN wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.2.0.zip
RUN unzip libtorch-cxx11-abi-shared-with-deps-1.2.0.zip -d /opt/

# Install CMake
RUN wget -q https://cmake.org/files/v3.14/cmake-3.14.6-Linux-x86_64.sh -O /cmake-3.14.6-Linux-x86_64.sh
RUN mkdir /opt/cmake
RUN sh /cmake-3.14.6-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
RUN cmake --version

# Build Singularity Trainer server
## Install dependencies
RUN apt-get update && apt-get install -y \
    libglu1-mesa-dev \
    python-dev \
    xorg-dev
## Copy files
RUN mkdir /app
WORKDIR /app
COPY . .
## Run build
RUN mkdir build
WORKDIR /app/build
RUN cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="/opt/libtorch" ..
RUN make -j8 Server


# Main image
FROM ubuntu:19.04 as main

# Install apt dependencies
RUN apt-get update && apt-get install -y \
    libcurl4 \
    libx11-6

# Copy over shared libraries
RUN mkdir /app
COPY --from=builder /opt/libtorch/lib/* /app/

# Copy over executable
COPY --from=builder /app/build/Server /app/Server

# Setup LD_LIBRARY_PATH
WORKDIR /app
ENV LD_LIBRARY_PATH=.

# Run server on container start
CMD [ "catchsegv", "./Server", "--agones", "-o", "hai" ]