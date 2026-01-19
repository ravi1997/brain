# Baseline Report - Brain Replica

## Project Overview
A complex cognitive engine simulation with a C++ backend and a React/Node.js frontend. It features a multi-port communication system and multi-layered persistence (Postgres/pgvector, SQLite, Redis).

## Build Status
- **Backend (C++)**: 
    - Build System: CMake
    - Local Build: **FAILED** (Missing `libpq` in development environment).
    - Container Build: Likely working (verified via `docker-compose config` and Dockerfile review).
    - GitHub Actions: CI defined in `.github/workflows/ci.yml`.
- **Frontend (Node/React)**:
    - Build System: npm/vite
    - Build: **SUCCESS** (Verified via `npm run build`).

## Test Status
- **Backend**: Extensive test suite in `tests/` (43+ files). Integration with GTest/CTest.
- **Frontend**: Vitest tests defined, Playwright E2E tests defined.

## Modules & Responsibilities
- `Brain`: Core coordinator of cognitive functions.
- `DNN`: Deep Neural Network for pattern recognition and learning.
- `CognitiveEngine`: Higher-level reasoning and RL logic.
- `MemoryStore`: In-memory and persistent storage abstraction.
- `PostgresStorage`: Vector-enabled persistence using `pgvector`.
- `BrainServer`: Multi-port TCP server management (Ports 9001-9014).
- `Web Dashboard`: React-based UI for monitoring and interaction.
- `WebSocket Proxy`: Node.js middleware for bridging UI to C++ backend.

## APIs & Interfaces
- **Port 9001**: Dashboard (JSON Status/Logs)
- **Port 9002**: Emotions (Broadcasting state)
- **Port 9005**: Chat (Interative text loop)
- **Port 9009**: Admin (Commands: save, reset, research, etc.)
- **Port 9013/9014**: Afferent/Efferent (Sensory input/Motor output)

## Data Models
- `brain_kv_store`: Key-Value storage with 384-dimensional vector embeddings for semantic search.

## Security Posture
- **Transport**: Raw TCP/WebSockets (Internal trusted network assumed).
- **Authentication**: No explicit auth found in core servers; `auth_system.hpp` exists but needs review.
- **Docker**: Root user used in Dockerfile (Security Gap).
- **Secrets**: Database passwords hardcoded in `docker-compose.yml` and `ci.yml`.

## Documentation Gaps
- No high-level architecture diagram.
- API protocol details (binary vs JSON) are undocumented.
- Setup guide for local development (outside Docker) is missing dependencies list.
