#!/bin/bash
set -e

# Find the container ID
CONTAINER_ID=$(docker compose ps -q brain)

if [ -z "$CONTAINER_ID" ]; then
    echo "Brain is not running. Try ./start.sh first."
    exit 1
fi

echo "Connecting to Brain Chat (Port 9005)... (Press Ctrl+C to exit)"
nc localhost 9005

