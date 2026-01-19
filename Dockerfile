FROM debian:bookworm

# Install build essentials and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    libtbb-dev \
    libsqlite3-dev \
    libhiredis-dev \
    libpq-dev \
    git \
    curl \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy project files
COPY . .

# Expose Neural Ports
# 9001: Dashboard, 9002: Emotions, 9003: Logs, 9005: Chat
# 9013: Input (Afferent), 9014: Output (Efferent)
EXPOSE 9001-9014

CMD ["./build/brain_replica"]
