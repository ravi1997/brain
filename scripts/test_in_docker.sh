#!/bin/bash
# Run tests inside the Docker container
# Usage: ./test_in_docker.sh

echo "Running tests in ephemeral container..."
# Use 'run' to execute a specific command in a new container, overriding the default command
# This avoids race conditions with the default container startup
docker-compose run --rm brain bash -c "mkdir -p build && cd build && cmake .. && make brain_tests && ./tests/brain_tests"
