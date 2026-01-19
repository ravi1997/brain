# Plan Output Contract: Folder Layout

## Directory Structure

All output files MUST be saved to these exact relative paths in the project root:

- `plans/SRS/`: Requirements documents.
- `plans/Architecture/`: Diagrams, models, and system design docs.
- `plans/Milestones/`: Roadmap and high-level phase definitions.
- `plans/Features/`: Detailed implementation plans for single tasks.
- `plans/Tests/`: Test plans, results, and bug reports.
- `plans/Security/`: Threat models and audit results.
- `plans/DevOps/`: Docker configurations, CI/CD plans, and deployment strategies.
- `plans/Release/`: Release notes and review summaries.
- `plans/Questions/`: Questionnaires for the User.

## Forbidden Locations

- Do NOT save plans inside `agent/` (unless it is a lifecycle state).
- Do NOT save plans inside `src/`.
- Do NOT save plans in `/tmp/`.
