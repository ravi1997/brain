#!/bin/bash
set -e

# Find the container ID
CONTAINER_ID=$(docker compose ps -q brain)

if [ -z "$CONTAINER_ID" ]; then
    echo "Brain is not running. Try ./start.sh first."
    exit 1
fi

echo "Connecting to Brain... (Press Ctrl+P, Ctrl+Q to detach without stopping)"
docker attach $CONTAINER_ID
