# Brain Simulation Project

## Project Overview

A sophisticated C++ brain simulation system that implements a cognitive architecture with multiple interconnected neural network modules. This project simulates various aspects of brain functionality including sensory processing, memory consolidation, pattern recognition, knowledge abstraction, and adaptive learning.

### Key Features

- **Advanced Neural Network Architecture**: Implements neural networks with synaptic plasticity, Hebbian learning, and homeostatic regulation
- **Multi-Module Cognitive System**: Includes Vision, Memory, Attention, World Model, Value, and Policy modules
- **Dynamic Learning Phases**: Supports Acquisition, Consolidation, Retrieval, and Testing phases based on input characteristics
- **Knowledge Hierarchy Management**: Maintains and evolves a knowledge graph with concepts, relationships, and confidence levels
- **Adaptive Learning**: Implements real-time learning curve optimization with error correction and selective forgetting
- **Conflict Resolution**: Detects and resolves contradictions in knowledge, with sophisticated reasoning capabilities

## Architecture

The system is organized into several core components:

### Core Modules
- `NeuralNetwork`: Core deep neural network implementation with plasticity mechanisms
- `BrainEngine`: Central coordination system that manages multiple brain modules
- `CognitiveBrain`: Basic cognitive architecture for reinforcement learning tasks
- `AdvancedBrainSimulation`: Enhanced brain simulation with sophisticated learning algorithms

### Cognitive Components
- `SensoryProcessingModule`: Processes sensory inputs
- `MemoryConsolidationModule`: Strengthens and organizes learned information
- `PatternRecognitionModule`: Identifies patterns in input data
- `KnowledgeAbstractionModule`: Creates hierarchical knowledge representations

## Building and Running

### Prerequisites
- C++20 compatible compiler
- CMake 3.20 or higher

### Build Commands
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)
```

### Executables
- `brain`: Main interactive brain simulation application
- `test_brain`: Unit tests for the brain simulation system

### Running
```bash
# Run the main brain simulation
./build/brain

# Run the tests
./build/test_brain
```

## Usage

### Interactive Brain Simulation
The main program provides an interactive interface where:
- Type statements to learn (e.g., "mango is a fruit")
- Type questions to retrieve knowledge (e.g., "what is mango?")
- Type "status" to check current learning phase
- Type "knowledge" to see stored concepts
- Type "quit" to exit

### Programmatic Usage
The system can be integrated into other applications by including `brain.hpp` and using the `AdvancedBrainSimulation` class.

## Development Conventions

### Code Style
- C++20 features are utilized throughout
- Consistent use of `brain::Tensor` for vector operations
- RAII principles for resource management
- Modern C++ practices with smart pointers
- Parallel execution with `std::execution` where possible

### Testing
- Comprehensive test suite in `test_brain_simulation.cpp`
- Unit tests cover tensor operations, brain functionality, neural networks, and cognitive modules
- Run tests with `./build/test_brain`

### Key Data Structures
- `Tensor`: `std::vector<double>` for neural computations
- `Decision`: Contains action, logits, probabilities, and value estimates
- `Experience`: Stores state transitions for learning
- `KnowledgeNode`: Represents concepts in the knowledge hierarchy

## Advanced Features

### Neural Plasticity
- Synaptic pruning and consolidation
- Hebbian learning mechanisms
- Homeostatic regulation
- Eligibility traces for temporal credit assignment

### Learning Optimization
- Real-time learning curve optimization
- Error correction mechanisms
- Adaptive weight adjustment
- Selective forgetting based on importance and recency

### Cognitive Architecture
- Multi-layered processing with specialized modules
- Dynamic phase detection and transition
- Context-based processing and attention mechanisms
- Memory consolidation with configurable frequency

This project represents a sophisticated approach to artificial cognitive systems with biologically-inspired mechanisms for learning, memory, and adaptation.