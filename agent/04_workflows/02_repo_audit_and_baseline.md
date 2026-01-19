# Workflow: Repo Audit and Baseline

## Purpose

Verify the health of an existing project before making changes.

## Inputs

- Existing project files.

## Procedure

1. **Adopt Profile**: `profile_tester.md`.
2. **Build Test**: Attempt to build the project.
3. **Test Run**: Execute existing tests.
4. **Code Quality**: Run linters if available.
5. **Docker Validation**: If Dockerfile exists, run `docker build` to verify it works. If docker-compose.yml exists, run `docker compose config` to validate syntax.
6. **CI/CD Check**: If `.github/workflows/` exists, validate workflow syntax using `actionlint` or manual review. Check if workflows are up-to-date with current build/test commands.
7. **Container Security Scan**: If Docker is present, check for pinned base images, non-root user, and absence of secrets in Dockerfile.
8. **Git Snapshot**: Record current HEAD hash in `PROJECT_STATE.md`.
9. **Report Card**: Create `plans/Architecture/BASELINE_REPORT.md` noting existing bugs/debt, Docker status, and CI/CD status.

## STOP Condition

- Baseline report is saved.

## Related Files

- `02_detection/detect_repo_health.md`
