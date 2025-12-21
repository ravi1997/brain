#!/bin/bash

# Configuration
KNOWLEDGE_FILE="knowledge.txt"
CONTAINER_NAME="brain-1" # As usually named by docker compose
WAIT_TIME=5 # Seconds to wait between topics to be polite to Wikipedia

echo "Starting Knowledge Training..."

if [ ! -f "$KNOWLEDGE_FILE" ]; then
    echo "Error: $KNOWLEDGE_FILE not found!"
    exit 1
fi

# Ensure brain is running
if [ "$(docker compose ps --services --filter "status=running" | grep brain)" == "" ]; then
    echo "Brain container is not running. Starting it in detached mode..."
    docker compose up -d brain
    echo "Waiting for brain to initialize..."
    sleep 5
fi

# We need to find the actual container name (sometimes it's brain-brain-1 or brain_brain_1)
CONTAINER_ID=$(docker compose ps -q brain)

if [ -z "$CONTAINER_ID" ]; then
    echo "Could not find Brain container ID."
    exit 1
fi

echo "Found Brain container: $CONTAINER_ID"

# Loop through topics
while IFS= read -r topic || [ -n "$topic" ]; do
    # Skip empty lines
    if [ -z "$topic" ]; then continue; fi
    
    echo "Teaching Brain about: $topic"
    
    # Send command to container input
    # 'docker attach' takes stdin, but it's tricky to script.
    # Better approach: We modify main.cpp to accept commands differently? 
    # Or, simpler: We restart the container in interactive mode within this script? 
    # Actually, easiest way is to use 'docker exec' if main.cpp read from a file, but it reads from stdin.
    
    # WORKAROUND:
    # We will use a python script to drive the interactive session if we want to do it in one go.
    # BUT, 'run_command' tool suggests we can send input to running command. 
    # The user wants a script *they* can run.
    
    # Option B: Use a pipe into docker attach?
    # echo "research $topic" | docker attach $CONTAINER_ID
    # This detaches instantly.
    
    # Option C (Robust): Restart the brain with input piped from a generated sequence.
    # This is what we will do for batch training.
    
    # Append "exit" to the sequence ensures it finishes.
done < "$KNOWLEDGE_FILE"

# Re-thinking: Piping to docker attach is flaky.
# Better Solution:
# We will create a clean "input.txt" that contains all commands, then cat it into the container.
# This requires stopping the current interactive session if any.

echo "Generating training plan..."
rm -f training_input.txt
while IFS= read -r topic || [ -n "$topic" ]; do
    if [ ! -z "$topic" ]; then
        echo "research $topic" >> training_input.txt
        # Add a small pause logic inside brain? No, we handle it by just queuing commands.
        # The main loop processes one getline per iteration.
    fi
done < "$KNOWLEDGE_FILE"
echo "exit" >> training_input.txt

echo "Feeding knowledge to the Brain..."
# We run a temporary container for training to not interfere with your main session
docker compose run --rm -T brain < training_input.txt

echo "Training complete. Memories stored in DB."
