#!/bin/bash
set -e
echo "Running Unit Tests..."
docker compose run --rm brain bash -c "mkdir -p build && cd build && cmake .. && make && ./tests/brain_tests"
