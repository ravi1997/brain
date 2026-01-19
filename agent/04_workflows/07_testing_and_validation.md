# Workflow: Testing and Validation

## Purpose

Exhaustive verification of the system's correctness.

## Inputs

- Implemented features.

## Procedure

1. **Adopt Profile**: `profile_tester.md`.
2. **Suite Execution**: Run all unit, integration, and E2E tests.
3. **Docker Test**: If Dockerfile exists, build image and run tests inside container to ensure environment parity.
4. **Compose Integration Test**: If docker-compose.yml exists, run `docker compose up` and execute integration tests against running services.
5. **CI Pipeline Validation**: If GitHub Actions exist, ensure all CI jobs pass locally before pushing.
6. **Manual Check**: Perform the steps in `04_workflows/03_srs_generation.md` Acceptance Criteria.
7. **Regression**: Ensure old features still work.
8. **Perf Check**: (Optional) Run basic load tests.
9. **Report**: Update `agent/09_state/TEST_STATE.md` and save `plans/Tests/REPORT.md`.

## STOP Condition

- Test suite has 100% pass rate.

## Related Files

- `11_rules/testing_rules.md`
