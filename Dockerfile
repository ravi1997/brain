FROM debian:bookworm

# Install build essentials
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    libtbb-dev \
    git \
    curl \
    && rm -rf /var/lib/apt/lists/*

COPY words.txt /usr/share/dict/words

# Set GCC 13 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 --slave /usr/bin/g++ g++ /usr/bin/g++-13

WORKDIR /app

# Copy project files
COPY . .

# Build
RUN mkdir build && cd build && cmake .. && make

CMD ["./build/brain_replica"]
