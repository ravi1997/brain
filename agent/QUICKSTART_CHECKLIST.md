# Quickstart Checklist

## Phase 0: Setup

- [ ] `agent/` folder is in the project root.
- [ ] Assistant has read `agent/AGENT_MANIFEST.md`.
- [ ] `plans/` directory exists (or is created by Agent).

## Phase 1: Intake

- [ ] Run `02_detection/detect_existing_vs_new.md`.
- [ ] Run `02_detection/detect_stack_signals.md`.
- [ ] `09_state/PROJECT_STATE.md` initialized with "State: BOOTSTRAP".

## Phase 2: Definition

- [ ] `profile_analyst_srs.md` activated.
- [ ] `plans/SRS/` populated with functional requirements.
- [ ] `gate_global_docs.md` passed.

## Phase 3: Design

- [ ] `profile_architect.md` activated.
- [ ] `plans/Architecture/` populated with system diagrams.
- [ ] `gate_global_quality.md` passed.

## Phase 4: Implementation

- [ ] `profile_implementer.md` activated.
- [ ] Backlog created in `agent/09_state/BACKLOG_STATE.md`.
- [ ] Feature branches/commits following `11_rules/repo_hygiene_rules.md`.

## Phase 5: Verification

- [ ] `profile_tester.md` activated.
- [ ] `plans/Tests/` contains automated and manual test results.
- [ ] `gate_global_security.md` passed.

## Phase 6: Release

- [ ] `profile_release_manager.md` activated.
- [ ] `agent/09_state/RELEASE_STATE.md` updated.
- [ ] `gate_global_release.md` passed.
