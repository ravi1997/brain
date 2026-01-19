# Software Requirements Specification (SRS) - Brain Replica

## 1. Introduction
The Brain Replica is an autonomous cognitive agent designed to simulate human-like thought processes, emotions, and memory. It serves as a testbed for advanced neural architectures and autonomous behaviors.

## 2. System Overview
The system consists of a C++ core responsible for neural processing and a React/Node.js web interface for monitoring and interaction.

## 3. Functional Requirements

### 3.1 Neural Processing (Implemented)
- **R1.1**: Use a multi-region neural architecture (Language Encoder, Decoder, Memory Center, Cognitive Center).
- **R1.2**: Support online learning via synaptic plasticity (reinforcement of active pathways).
- **R1.3**: Provide a Bag-of-Words and N-GRAM based language processing loop.

### 3.2 Memory & Persistence (Implemented)
- **R2.1**: Support Short-Term Memory (STM) via a conversation context buffer.
- **R2.2**: Support Long-Term Memory (LTM) via associative RAG-lite retrieval using PostgreSQL and pgvector.
- **R2.3**: Cache frequent queries using Redis for performance (Optional/Supported).

### 3.3 Emotional & Biological Dynamics (Implemented / Partially Implemented)
- **R3.1**: Simulate core emotions (Happiness, Sadness, Anger, Boredom).
- **R3.2**: Implement Homeostasis with energy decay and sleep cycles (Partially Implemented).
- **R3.3**: Modulate behavioral responses based on emotional state.

### 3.4 Skill & Task Management (Implemented)
- **R4.1**: Define and teach specific "Neural Skills" via the `Learn:` command.
- **R4.2**: Execute skills via the `Do:` command.
- **R4.3**: Prioritize and execute asynchronous tasks via `PackageManager`.

### 3.5 Sensory Integration (Implemented)
- **R5.1**: Support multi-modal sensory input (Vision, Audio, Clock, Spatial, Tactile).
- **R5.2**: Provide high-speed afferent (input) and efferent (output) ports for hardware integration (ROS 2 Bridge).

### 3.6 User Interface (Implemented)
- **R6.1**: Provide a real-time web dashboard showing logs, emotions, and knowledge graph.
- **R6.2**: Support a text-based chat interface proxied via WebSockets.

## 4. Non-Functional Requirements
- **N1 Performance**: C++ backend for high-frequency neural updates.
- **N2 Reliability**: Crash reporting and periodic state saving.
- **N3 Scalability**: Vector indexing (HNSW) for large memory stores.

## 5. Feature Status Matrix (From Reality)

| Feature ID | Description | Status |
|---|---|---|
| F-CORE-01 | Synaptic Plasticity | Implemented |
| F-MEM-01 | Vector LTM (Postgres) | Implemented |
| F-MEM-02 | Redis Caching | Implemented |
| F-EMO-01 | Emotional Modulation | Implemented |
| F-BIO-01 | Circadian Rhythm / Sleep | Partially Implemented |
| F-SKL-01 | Neural Skill teaching | Implemented |
| F-TASK-01 | Task Manager | Implemented |
| F-NET-01 | Multi-port Server | Implemented |
| F-SEC-01 | User Authentication | Missing |
| F-HARD-01 | ROS 2 Integration | Implemented (Bridge) |
