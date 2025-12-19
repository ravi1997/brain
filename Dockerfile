FROM ubuntu:24.04

# Install build essentials and modern C++ compiler
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++-13 \
    libtbb-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set GCC 13 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 --slave /usr/bin/g++ g++ /usr/bin/g++-13

WORKDIR /app

# Copy project files
COPY . .

# Build
RUN mkdir build && cd build && cmake .. && make

CMD ["./build/brain_replica"]
