# Workflow: Deep Feature Implementation Loop

## Purpose

The primary execution engine for building validated, high-quality features.

## Inputs

- A "Todo" task from the Backlog with a clear ID (e.g., T-001).

## Procedure (Adopt `profile_implementer.md`)

### Phase 1: Scaffolding & Design

1. **Kickoff**: Update `BACKLOG_STATE.md` to "In Progress".
2. **Requirement Analysis**: Read the linked SRS section and all relevant User Stories.
3. **Drafting**: Use `07_templates/feature/feature_kickoff.md` to map out:
   - Affected files and new modules.
   - External dependencies needed.
   - Potential side-effects on existing features.
4. **Sign-off**: Present the plan to the User for approval before writing any code.

### Phase 2: Implementation & Self-Correction

1. **Atomic Development**: Divide the task into small commits (one function or logical unit at a time).
2. **Build-Fast Loop**:
   - Write code for a single logical unit.
   - Run the build/compile command (`npm run build`, `mvn compile`, etc.).
   - If it fails: Read the error, fix it immediately, and document the error in `agent/09_state/PROJECT_STATE.md` if it represents a recurring logic hole.
3. **Internal Testing**:
   - Write a unit test for the new unit.
   - Run the test.
   - **Recursion**: If the test fails, do NOT move to the next unit until it passes and is refactored for clarity.
4. **Clean Code Audit**: If a single function exceeds 30 lines or 5 levels of nesting, refactor it immediately into smaller private methods.

### Phase 3: Recursive Verification (Adopt `profile_tester.md`)

1. **Local Integrity**: Run all existing tests in the same directory.
2. **Global Stability**: Run the full project test suite to catch regressions.
3. **Docker Build Check**: If Dockerfile exists, run `docker build` to ensure the feature doesn't break containerization.
4. **Compose Validation**: If docker-compose.yml exists, run `docker compose up --build` in test mode to verify services start correctly.
5. **CI/CD Sync**: If new tests or build steps were added, update `.github/workflows/ci.yml` to include them. Validate workflow syntax.
6. **Security Check**: Run `06_skills/security/` to ensure no keys were leaked and no vulnerable libraries were added.
7. **Performance Gate**: Verify that the time/memory complexity is within the limits defined in `plans/SRS/non_functional_reqs.md`.

### Phase 4: Finalization (Adopt `profile_rule_checker.md`)

1. **Gate Pass**: Verify `05_gates/global/gate_global_quality.md`.
2. **Sync**: Update all related documentation (API docs, README).
3. **Commit**: Use Conventional Commits (`feat(auth): ...`) and update `BACKLOG_STATE.md` to "DONE".
4. **Post-Mortem**: Run `06_skills/metacognition/skill_self_audit.md` to see if the workflow itself can be improved.

## Failure Modes & Handling

- **Cyclic Conflict**: If fixing one bug breaks another, STOP and adopt `profile_architect.md` to review the component design.
- **Deadlock**: If the build fails and the fix is unknown after 3 attempts, revert to the last commit and request User guidance.

## Related Files

- `11_rules/code_style_rules.md`
- `05_gates/global/gate_global_quality.md`
- `06_skills/metacognition/skill_self_audit.md`
