# Error Handling

## Purpose

Defines how the Agent reacts to tool failures, environment issues, and logic errors.

## Recovery Procedures

### 1. Tool Execution Failure

- **Symptoms**: `run_command` returns non-zero exit code.
- **Procedure**:
  1. Read the `stderr`.
  2. Search for the error string in the project (to see if it has happened before).
  3. Attempt 1 automated fix (e.g., missing dependency install).
  4. If failure persists, escalate to the User with the full log.

### 2. Context Overflow

- **Symptoms**: AI starts losing track of earlier instructions or state.
- **Procedure**:
  1. Trigger a "State Refresh".
  2. Summarize `09_state/` into a single "Context Shot".
  3. Close unnecessary files/terminals.

### 3. Infinite Loops

- **Symptoms**: Agent repeating the same command 3+ times with the same result.
- **Procedure**:
  1. **STOP**.
  2. Flag the loop to the User.
  3. Identify the "Cycle Break" (a different approach or manual intervention).

### 4. Permission Denied

- **Symptoms**: Filesystem or Network access errors.
- **Procedure**:
  1. Verify the path is within `01_scope_and_limits.md`.
  2. Request the User to grant permissions via terminal commands or manual chmod.

## Error Codes

- `E-DET-001`: Stack detection failed (Too ambiguous).
- `E-GATE-500`: Gate failure (Quality threshold not met).
- `E-SEC-666`: Security violation (Leak detected).

## Related Files

- `04_error_handling.md`
- `05_gate_failure_playbook.md`
