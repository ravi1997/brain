# System Procedure: Context & Memory Management

## Purpose

Prevents the AI Agent from "forgetting" requirements or making conflicting decisions during long-running sessions where the context window might overflow.

## Symptoms of Context Decay

- The Agent asks a question that was already answered 10 turns ago.
- The Agent "forgets" it is in the middle of a refactoring loop.
- The Agent fails to follow specialized rules from `11_rules/`.

## Mandatory Memory Routine (Adopt `profile_rule_checker.md`)

1. **Checkpointing**: Every 10 tool calls, the Agent must re-read `agent/09_state/PROJECT_STATE.md` to refresh its sense of direction.
2. **Context Compression**: If the conversation becomes very long, the Agent must:
   - Summarize the "Current Goal" and "Significant Obstacles" into a single message.
   - Close all background terminals that are no longer needed.
   - Request the user to "Clear History/Start New Session" if the model's internal limits are reached, passing a "State Dump" file to the new session.
3. **State Syncing**: Every successful file edit MUST be followed by an update to the relevant state file (SRS, BACKLOG, or TEST).

## State Dump Procedure

If the Agent detects a new session (e.g., no conversation history but project files exist):

1. **Bootup**: Read `agent/AGENT_MANIFEST.md` and `agent/09_state/PROJECT_STATE.md`.
2. **Re-Detection**: Run a shallow `02_detection/` to ensure the filesystem hasn't changed externally.
3. **Resume**: Announce the last active task from the backlog and ask for confirmation to continue.

## Related Files

- `09_state/PROJECT_STATE.md`
- `00_system/02_response_format.md`
