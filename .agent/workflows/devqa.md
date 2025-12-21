---
description: A continuous loop where the agent implements C++ features from the backlog and verifies them using run_tests.sh.
---

# Multi-Agent Continuous Development Workflow

## Overview
This workflow sets up a continuous improvement loop using autonomous agents that work off a `backlog.md` file. The goal is to simulate a "Dev Team" where tasks are picked up, implemented, tested, and marked as complete automatically.

## Agents

### 1. Developer Agent (`agents/developer.py`)
- **Role**: Implements features and fixes.
- **Workflow**:
    1. Reads `backlog.md`.
    2. Identifies a `[ ]` item under "High Priority".
    3. Changes the item to `[/]` (In Progress).
    4. **Action**: Performs code modifications. 
       - *Current Implementation*: Uses heuristics to find "safe" files to modify (e.g., tweaking constants in `brain.cpp` or adding comments) to simulate work.
    5. Commits changes to git with a descriptive message referencing the backlog item.
    6. Updates `backlog.md` to move item to `[x]` (Completed) - *Note: In a real flow, this might wait for the Tester, but for simplicity, the Developer marks it done or valid for testing.* -> *Correction*: The Developer submits for testing. The Tester marks it done.

### 2. Tester Agent (`agents/tester.py`)
- **Role**: Validates changes.
- **Workflow**:
    1. Checks if there have been recent commits or if an item is "In Progress".
    2. Runs the build/test suite (`docker compose build`, `npm test`, etc.).
    3. **Pass**:
        - Marks `[/]` items in `backlog.md` as `[x]` (Completed).
        - Logs success.
    4. **Fail**:
        - Reverts the last commit (or notifies Developer).
        - Changes `[/]` item back to `[ ]` (Todo) or Marks as `[!]` (Failed).
        - Logs failure.

## Continuous CI Loop (`agents/ci_loop.py`)
- Orchestrates the agents.
- Runs in a loop:
    ```python
    while True:
        developer.work()
        tester.validate()
        sleep(60)
    ```

## Usage
To start the autonomous team:
```bash
python3 agents/ci_loop.py
```
