# Workflow: Intake and Context Build

## Purpose

Gather enough information to start making high-value decisions.

## Inputs

- User command (Run New or Run Existing).

## Procedure

1. **Adopt Profile**: `profile_rule_checker.md`.
2. **Scan Directory**: Get a recursive file list.
3. **Run Detection**:
   - `02_detection/detect_existing_vs_new.md`
   - `02_detection/detect_project_type.md`
   - `02_detection/detect_stack_signals.md`
4. **Knowledge Extraction**: Read key files (README, configs).
5. **Interview User**: Ask 3-5 high-level questions about the goal.
6. **Initialize State**: Create `agent/09_state/PROJECT_STATE.md`.

## STOP Condition

- State is initialized and Stack is identified.

## Related Files

- `01_entrypoints/run_existing_project.md`
