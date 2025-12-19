#!/bin/bash
set -e

# Build the Docker image
echo "Building Docker image..."
docker build -t brain_replica .

# Run the container
echo "Running Brain Replica..."
docker run --rm -it brain_replica
