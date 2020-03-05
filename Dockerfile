FROM gcc:9 as builder

# Install PyTorch
RUN wget -O libtorch.zip https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-1.4.0%2Bcpu.zip
RUN unzip libtorch.zip -d /opt/

# Install CMake
RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.16.0-rc3/cmake-3.16.0-rc3-Linux-x86_64.sh -O /cmake-install.sh
RUN mkdir /opt/cmake
RUN sh /cmake-install.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
RUN cmake --version

# Build AI: Artificial Insentience server
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
COPY --from=builder /opt/libtorch/lib/*.so* /app/

# Copy over executable
COPY --from=builder /app/build/Server /app/Server

# Setup LD_LIBRARY_PATH
WORKDIR /app
ENV LD_LIBRARY_PATH=.

# Run server on container start
CMD [ "catchsegv", "./Server", "--agones", "-o", "hai" ]