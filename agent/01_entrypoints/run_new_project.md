# Entrypoint: Run New Project

## Purpose

The primary trigger when starting a project from an empty or nearly-empty directory.

## Inputs

- High-level project description from the User.
- Desired technology stack (optional, Agent can suggest).

## Procedure

1. **Adopt Profile**: Switch to `03_profiles/profile_analyst_srs.md`.
2. **Define Scope**: Execute `04_workflows/03_srs_generation.md`.
3. **Choose Stack**:
   - If User specified: Verify feasibility via `02_detection/stack_fingerprints/`.
   - If not specified: Present 3 options based on project type (Web, API, CLI, etc.).
4. **Initialize Structure**: Create the basic folder layout including `plans/` and basic stack files (e.g., `init.py`, `package.json`).
5. **Setup Gates**: Verify `05_gates/gate_global_docs.md`.
6. **Create Backlog**: Transition to `04_workflows/05_milestones_and_backlog.md`.

## Stop Conditions

- User rejects the proposed stack or SRS.
- Directory is not writable.

## Failure Modes

- **Requirements Gap**: If the project description is too vague, the Agent must present a questionnaire from `06_skills/analysis/`.

## Related Files

- `04_workflows/03_srs_generation.md`
- `04_workflows/05_milestones_and_backlog.md`
