# SRS: Non-Functional Requirements - Brain Replica

## 1. Performance
- **Neural Propagation**: < 50ms latency for a single thought inference.
- **Memory Retrieval**: < 200ms for semantic search in Postgres (up to 10k records).
- **Concurrency**: Support at least 10 simultaneous TCP port connections without degradation.

## 2. Security
- **Data Isolation**: All brain state files (`state/*.json`) shall have restricted file permissions.
- **Port Security**: Administrative ports (9009) shall be protected by a configurable access token (Planned).
- **User Safety**: Continuous output running (Efferent) shall be rate-limited to 5 messages/sec.

## 3. Reliability
- **Persistence**: Auto-save of synaptic weights every 10 minutes of activity.
- **Crash Recovery**: `CrashReporter` shall log stack traces to `state/logs/` and attempt a clean restart of the TCP servers.
- **Network**: Passive reconnect logic for the WebSocket proxy.

## 4. Scalability
- **Horizontal Scaling**: Neural regions can be shifted to separate processes or hardware accelerators (CpuAccelerator/GpuAccelerator placeholders).
- **Storage**: Postgres with HNSW index to support millions of memory records.
