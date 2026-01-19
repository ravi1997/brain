# System Procedure: Graceful Degradation & Failure Handling

## Purpose

Defines how the Agent OS should behave when encountering unrecoverable errors or reaching the limits of its capabilities.

## Failure Categories

### 1. Tool Failure (Cannot Execute Command)

**Symptoms**: `run_command` returns error, file system is read-only, network is down.

**Response**:

1. **Retry Once**: Attempt the command one more time after a 2-second delay.
2. **Escalate**: If retry fails, report the exact error to the User with:
   - The command that failed.
   - The error message.
   - Suggested manual remediation steps.
3. **State Preservation**: Save the current task state to `agent/09_state/RECOVERY_CHECKPOINT.md`.

### 2. Knowledge Gap (Unknown Stack or Pattern)

**Symptoms**: No matching fingerprint in `02_detection/stack_fingerprints/`, unfamiliar error message.

**Response**:

1. **Web Search**: Use `search_web` to find documentation or Stack Overflow answers.
2. **User Query**: Ask the User: "This appears to be [X]. Can you confirm and provide documentation links?"
3. **Learning**: Once resolved, create a new fingerprint file in `02_detection/stack_fingerprints/custom_[name].md`.

### 3. Logical Impossibility (Contradictory Requirements)

**Symptoms**: Cannot satisfy all constraints simultaneously.

**Response**:

1. **Route to Scenario**: Use `01_entrypoints/scenarios/scenario_conflict_resolution.md`.
2. **Do Not Guess**: Never silently choose one requirement over another.

### 4. Resource Exhaustion (Out of Memory, Disk Full)

**Symptoms**: System errors, slow performance, tool timeouts.

**Response**:

1. **Cleanup**: Close unnecessary terminals, delete temporary files in `/tmp/`.
2. **Report**: Notify the User of resource constraints.
3. **Pause**: Do not continue implementation until resources are freed.

### 5. Context Overflow (Conversation Too Long)

**Symptoms**: Agent starts repeating itself, forgetting earlier decisions.

**Response**:

1. **Trigger**: Use `00_system/08_context_management.md` to compress state.
2. **State Dump**: Create a comprehensive summary in `agent/09_state/SESSION_SUMMARY.md`.
3. **User Handoff**: Request the User to start a new session and provide the state dump.

## General Principles

- **Fail Loudly**: Never silently ignore errors.
- **Preserve State**: Always save progress before reporting a failure.
- **User Autonomy**: Give the User clear options, not just "I can't do this".

## Related Files

- `00_system/04_error_handling.md`
- `00_system/08_context_management.md`
