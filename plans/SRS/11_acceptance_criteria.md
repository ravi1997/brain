# SRS: Acceptance Criteria - Brain Replica

## Milestone 1: Stabilization (Verification)
- **AC-1.1**: Application runs in Docker as user `brain`.
- **AC-1.2**: `cmake .. && make` works without manual intervention on a standard Debian environment (provided dependencies are installed).
- **AC-1.3**: All tests in `tests/` pass with `ctest`.

## Milestone 2: Neural Core (Verification)
- **AC-2.1**: Sending "hello" to Port 9005 results in a coherent response.
- **AC-2.2**: Teaching a skill via `Learn: test Input: a Output: b` allows subsequent retrieval via `Do: test Input: a`.
- **AC-2.3**: Injecting high sentiment words (e.g., "awesome") reflected as an increase in `happiness` on Port 9002.

## Milestone 3: Persistence (Verification)
- **AC-3.1**: Data remains accessible after a container restart (volumes).
- **AC-3.2**: Sequential recall: Tell the brain a secret, restart, and it should recall it using semantic search triggers.
