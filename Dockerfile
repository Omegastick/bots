FROM gcc:9 as builder

# Install PyTorch
RUN wget https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip
RUN unzip libtorch-shared-with-deps-latest.zip -d /opt/

# Install CMake
RUN wget -q https://cmake.org/files/v3.14/cmake-3.14.6-Linux-x86_64.sh -O /cmake-3.14.6-Linux-x86_64.sh
RUN mkdir /opt/cmake
RUN sh /cmake-3.14.6-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
RUN cmake --version

# Install Agones C++ SDK
WORKDIR /opt
RUN git clone --branch v0.12.0 --depth=1 https://github.com/googleforgames/agones.git
RUN mkdir /opt/agones/sdks/cpp/build
WORKDIR /opt/agones/sdks/cpp/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=./install
RUN cmake --build . --target install -j4

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
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="/opt/libtorch;/opt/agones/sdks/cpp/build/install" ..
RUN make -j4 Server


# Main image
FROM ubuntu:18.04

# Install apt dependencies
RUN apt-get update && apt-get install -y \
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
ENTRYPOINT [ "./Server", "--agones" ]