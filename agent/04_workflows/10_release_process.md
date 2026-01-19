# Workflow: Release Process

## Purpose

Package and finalize the project for end-user consumption.

## Inputs

- Current stable branch passing all Gates.

## Procedure

1. **Adopt Profile**: `profile_release_manager.md`.
2. **Documentation**: Update `CHANGELOG.md` and `README.md`.
3. **Versioning**: Increment version number based on SemVer rules.
4. **Tagging**: Create a Git tag (e.g., `v1.0.0`).
5. **Docker Images**: If Docker is used, build and tag production images with version number. Push to container registry if configured.
6. **GitHub Release**: Create a GitHub release from the tag, attaching release notes and any build artifacts.
7. **Artifacts**: Generate build artifacts (e.g., `dist/`, `.zip`).
8. **Release Notes**: Fill `07_templates/release/release_notes_template.md` and attach to GitHub release.
9. **CI/CD Trigger**: If release workflow exists (`.github/workflows/release.yml`), ensure it triggers on tag push.
10. **State update**: Set `PROJECT_STATE.md` to "State: RELEASED".

## STOP Condition

- All release artifacts are generated and notes are saved.

## Related Files

- `05_gates/global/gate_global_release.md`
