# Example: Feature Cycle

## 1. Pick Task

Agent reads `agent/09_state/BACKLOG_STATE.md`, picks `T-04: Implement Auth Logic`.

## 2. Kickoff

Creates `plans/Features/AUTH_LOGIC_PLAN.md`. Adopts `profile_implementer.md`.

## 3. Code

Writes `auth.py`.

## 4. Verify

Runs `pytest tests/test_auth.py`. Fixes one failure.

## 5. Peer Review

Adopts `profile_pr_reviewer.md`. Finds a missing docstring. Fixes it.

## 6. Done

Updates `BACKLOG_STATE.md` to `DONE`.
