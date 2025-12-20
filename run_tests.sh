#!/bin/bash
set -e

# Start the container using Docker Compose
# This uses the configuration in docker-compose.yml to mount the current directory
# and perform an incremental build inside the container.
echo "Starting Brain Replica with Docker Compose..."
docker compose up brain
