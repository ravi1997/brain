# Example: New Project Walkthrough

## Scenario

The User wants a new "To-Do List CLI" in Node.js.

## Steps

1. **Goal**: "I want a CLI tool for to-dos, local storage only."
2. **Detection**: Root is empty -> `run_new_project.md`.
3. **SRS**: Agent generates `plans/SRS/TODO_CLI_SRS.md` with features: `add`, `list`, `remove`.
4. **Architecture**: Agent decides on `commander` for CLI and `JSON` for storage.
5. **Backlog**: Creates 4 tasks: `Init repo`, `Implement storage`, `Add CLI commands`, `Packaging`.
6. **Execution**: Loops through tasks until M1 is complete.
7. **Gate**: Full doc and test pass.
