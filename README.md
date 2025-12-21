# Brain Replica

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![C++ Standard](https://img.shields.io/badge/standard-C%2B%2B20-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Docker](https://img.shields.io/badge/deployment-docker-blue)

**Brain Replica** is an advanced, experimental AI simulation engine designed to model a simplified biological brain. Unlike traditional LLMs, it features distinct cortical regions, plastic neural networks with Hebbian learning, and simulated emotional states, all running in a continuous autonomous loop.

## Key Features

*   **🧠 Plastic Neural Networks**: Custom engine (`dnn.hpp`) supporting backpropagation and Hebbian learning with homeostatic regulation.
*   **🏙️ Cognitive Architecture**:
    *   **Broca/Wernicke Areas**: Natural Language Processing.
    *   **Hippocampus**: Associative Memory & Consolidation.
    *   **Prefrontal Cortex**: Decision Making & Planning.
*   **⚡ Autonomous Agency**: Runs on a background thread (`Brain::automata_loop`), experiencing fatigue, boredom, and curiosity.
*   **💾 O(1) Memory Retrieval**: High-performance inverted index for instant memory access (`MemoryStore`).
*   **🚀 Redis Caching**: Integrated **Redis** layer for caching frequent cognitive queries.
*   **⚛️ Modern React Dashboard**: Real-time visualization of the brain's internal state, thoughts, and vitals using **React** + **Vite**.

## Tech Stack

*   **Core**: C++20
*   **Database**: SQLite (Persistent Memory)
*   **Caching**: Redis
*   **Frontend**: React, Vite, WebSocket
*   **Infrastructure**: Docker, Docker Compose

## Getting Started

### Prerequisites

*   [Docker](https://docs.docker.com/get-docker/) & Docker Compose

### Quick Start

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/brain-replica.git
    cd brain-replica
    ```

2.  **Start the System:**
    ```bash
    docker-compose up --build -d
    ```
    *This starts the Brain backend (port 9001-9010), Redis (6379), and the Web Dashboard (5000).*

3.  **Access the Dashboard:**
    Open [http://localhost:5000](http://localhost:5000) in your browser.

### Terminal Interaction

You can also interact directly via the terminal:

```bash
./connect.sh
```
*(Press `Ctrl+P`, then `Ctrl+Q` to detach)*

## Configuration

Tune the brain's personality in `config.json`:

```json
{
    "curiosity": 0.9,      // Probability of random research
    "playfulness": 0.6,    // Affects response tone
    "energy_decay": 0.02   // Fatigue rate
}
```

## Architecture

The system follows a modular architecture:

```mermaid
graph TD
    User[User / Web UI] <-->|WebSocket| Brain
    Brain[Brain Engine (C++)] <-->|Read/Write| Redis[Redis Cache]
    Brain <-->|Persist| SQLite[SQLite Memory DB]
    Brain -->|Process| Cortex[Cortical Regions]
    Cortex -->|Learn| DNN[Neural Network]
```

## Development

*   **Backend**: `src/` (C++20)
*   **Frontend**: `web/client/` (React)
*   **Tests**: `tests/` (GoogleTest)

### Running Tests (Docker)
```bash
./test_in_docker.sh
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
