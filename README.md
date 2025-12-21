# Brain Replica

**Brain Replica** is an experimental AI simulation engine written in C++20. It models a simplified biological brain with distinct cortical regions, plastic neural networks, and simulated emotional states.

## Key Features

*   **Plastic Neural Networks**: Implements a custom neural network engine (`dnn.hpp`, `dnn.cpp`) supporting both standard backpropagation and Hebbian learning with homeostatic regulation.
*   **Cognitive Architecture**: Divided into specialized regions:
    *   **Broca's Area**: Language encoding.
    *   **Wernicke's Area**: Language decoding.
    *   **Hippocampus**: Memory consolidation.
    *   **Prefrontal Cortex**: Cognitive processing and decision making.
*   **Simulated Autonomy**: The "Brain" runs on a background thread (`Brain::automata_loop`), experiencing boredom, fatigue, and curiosity. It will spontaneously "research" topics or "sleep" based on its energy levels.
*   **Durable Memory**: Uses **SQLite** (`brain_memories.db`) to persist thoughts, research, and interactions across sessions.
*   **Configurable Persona**: Personality traits (curiosity, playfulness, energy decay) are tunable via `config.json`.

## Getting Started

### Prerequisites

*   **Docker** and **Docker Compose**
*   (Optional) C++20 compiler if building locally without Docker.

## Usage

### 1. Start the Brain (Background)
Run this once to start the persistent brain process:
```bash
./start.sh
```

### 2. Talk to the Brain
To interact with the running brain:
```bash
./connect.sh
```
*Press `Ctrl+P`, then `Ctrl+Q` to detach without stopping.*

### 3. Stop
```bash
./stop.sh
```

### 4. Training (Batch)
To feed a list of topics:
```bash
./train.sh
```

### Configuration

You can tune the brain's behavior by editing `config.json`. Changes take effect upon restart.

```json
{
    "curiosity": 0.9,      // Likelihood of random research
    "playfulness": 0.6,    // Affects tone (future)
    "energy_decay": 0.02   // How fast it gets tired
}
```

## Development

### Project Structure

*   `src/`:
    *   `main.cpp`: Entry point and REPL.
    *   `brain.hpp/cpp`: High-level simulation logic and state management.
    *   `dnn.hpp/cpp`: Neural network engine implementation.
    *   `memory_store.hpp/cpp`: SQLite database abstraction.
*   `tests/`: a GoogleTest suite for validating core components.

### Running Tests

Unit tests run inside the Docker container to ensure environment consistency.

```bash
./test.sh
```

## Theory of Operation

The core learning mechanism uses **Anti-Hebbian** and **ABCD** (Activity-Based Decay) rules alongside gradient descent. The `PlasticLayer` adjusts weights not just based on error (supervisor) but also based on the correlation of input/output activity, simulating biological synaptic plasticity.

### License

Project is open for research and educational purposes.
