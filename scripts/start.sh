#!/bin/bash
set -e

echo "Starting Brain in background..."
docker compose up -d --build brain
echo "Brain is running in the background."
echo "Use ./connect.sh to talk to it."
echo "Use ./stop.sh to stop it."
