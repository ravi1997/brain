#!/bin/bash
set -e
if [[ "$1" == "--local" ]]; then
    echo "Running Unit Tests Locally..."
    mkdir -p build && cd build && cmake .. && make -j$(nproc) && ./tests/brain_tests
else
    echo "Running Unit Tests in Docker..."
    docker compose run --rm brain bash -c "mkdir -p build && cd build && cmake .. && make -j$$(nproc) && ./tests/brain_tests"
fi
