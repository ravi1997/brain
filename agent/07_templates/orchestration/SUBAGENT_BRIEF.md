# Sub-agent Brief: [Profile Name]

## Assignment Details

- **Role**: [Profile Name]
- **Orchestration ID**: [ID]
- **Task**: [Detailed sub-task description]

## Inputs (Context)

- **State Source**: `agent/09_state/[STATE_FILE].md`
- **Dependencies**: [Paths to outputs from previous sub-agents]
- **Scope**: [Specific files or directories to touch]

## Execution Constraints

- Use logic from: `agent/03_profiles/profile_[name].md`
- Follow workflow: `agent/04_workflows/[name].md`
- Enforce gates: `agent/05_gates/[name]/`

## Expected Deliverables

- [Artifact Path 1]
- [Artifact Path 2]
- [Final Output Summary]

## State Update Requirement

- MUST update `agent/09_state/[NAME].md` before completion.
