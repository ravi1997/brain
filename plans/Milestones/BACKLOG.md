# Backlog Definition - Brain Replica

## Milestone 1: Stabilization & Security Baseline (Priority: CRITICAL)
*Objective: Make the project safe to run and easy to install for external developers.*

- [x] **[STAB-01]** Rootless Docker Implementation.
- [ ] **[STAB-02]** C++ Env Var Support: Replace hardcoded DB credentials with `std::getenv`.
- [ ] **[STAB-03]** CMake Robustness: Fix `libpq` discovery and add version checks.
- [ ] **[STAB-04]** Setup Automation: Create `scripts/setup_dev.sh` to install OS dependencies.

## Milestone 2: Access Control & Neural Gates (Priority: HIGH)
*Objective: Protect the brain's internal state and interaction ports.*

- [ ] **[SEC-01]** Token Auth implementation in `TcpServer`.
- [ ] **[SEC-02]** Neural Gate: Restrict administrative commands (save, reset) to authenticated sessions only.
- [ ] **[SEC-03]** Dashboard Login: Add simple session management to the Node.js proxy.

## Milestone 3: Cognitive Maintenance & Biological Lifecycle (Priority: MEDIUM)
*Objective: Implement "biological" constraints like sleep and forgetting.*

- [ ] **[COG-01]** Consolidation Cycle: Thread to move/summarize STM into LTM during idle periods.
- [ ] **[COG-02]** Memory Decay: PostgreSQL stored procedure to prune low-significance entries.
- [ ] **[COG-03]** Circadian Regulation: Automate energy recovery via "Sleep" state triggered by ClockUnit.

## Milestone 4: Parallel Processing & Optimization (Priority: LOW)
*Objective: Hardware-level performance improvements.*

- [ ] **[PERF-01]** TBB Region Parallelism: Multithreaded execution in `dnn.cpp`.
- [ ] **[PERF-02]** Semantic Cache: Use Redis for O(1) retrieval of previously computed embeddings.
