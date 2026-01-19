# Example: Existing Project Walkthrough

## Scenario

The Agent is added to a 1-year-old Python Flask project.

## Steps

1. **Intake**: Agent reads `app.py`, finds `Flask` imports. Sets stack to `python_flask`.
2. **Audit**: Runs `pytest`. 2 tests fail. Logs this in `PROJECT_STATE.md` under "Health: DEGRADED".
3. **Task**: User says "Add a /status endpoint".
4. **SRS**: Agent creates `plans/SRS/SRS_STATUS_ENDPOINT.md`.
5. **Implementation**: Agent writes code in `app.py`, writes a new test `test_status()`.
6. **Gate**: Quality gate passes, security gate (no secrets) passes.
7. **Release**: Agent updates `CHANGELOG.md` and sets state to `ACTIVE`.
