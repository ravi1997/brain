# Tester Guide: Brain Project

## Test Objectives
The goal is to ensure the Brain simulation functions correctly, maintaining stability while simulating complex biological and emotional dynamics. We need to verify correctness of logic, system performance, and UI responsiveness.

## Test Environment
- **OS**: Linux (Ubuntu 22.04+)
- **Backend**: C++20 build environment
- **Frontend**: Node 18+ environment
- **Browser**: Latest Chrome/Firefox

## Test Data
- No external test data is required; the system generates its own initial state.
- **Mocking**: For unit tests, `IMemory` and `ISensor` interfaces should be mocked.

## Test Cases

| ID | Title | Prerequisites | Steps | Expected Result | Result (P/F) |
|----|-------|---------------|-------|-----------------|--------------|
| TC1 | Backend Startup | Build successful | Run `./build/brain` | Server starts on port 8080 | |
| TC2 | Emotional Reactivity | App running | Send "I hate you" | Anger/Sadness increases in UI | |
| TC3 | Hunger Decay | App running | Wait 5 minutes | Hunger bar increases | |
| TC4 | Memory Persistence | App running | Say "My name is Alice" -> Restart -> Ask "Who am I?" | Brain replies "Alice" | |
| TC5 | UI Connection | Backend running | Open `localhost:3000` | Dashboard loads, websocket connects | |

## Automation Coverage
- **Unit Tests**: Located in `tests/`. Run via CTest or `./test.sh`.
    - `test_emotion_logic.cpp`: Covers emotional state transitions.
    - `brain_integration_test.cpp`: End-to-end logic checks.
- **Integration Tests**: `tests/brain_integration_test.cpp` verifies component interaction.

## Manual Verification Steps
1.  **Visual Inspection**: Open the web UI and watch for jitter or rendering glitches in the radar chart.
2.  **Long-running Stability**: Leave the brain running for 1 hour to check for memory leaks or crashes.
3.  **Conversational coherence**: Chat for 5 minutes and subjectively evaluate response quality.

## Regression Strategy
- Run the full unit test suite (`./test.sh`) before every commit.
- Perform a manual "smoke test" (TC1 & TC5) before merging any PR.

---
**Lead Tester:** Antigravity Agent
**Status:** DRAFT
