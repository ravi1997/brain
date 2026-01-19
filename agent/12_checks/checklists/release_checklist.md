# Checklist: Release

## Purpose
This checklist ensures all steps for a production-ready release are COMPLETED.

## Verification Items
- [x] All unit tests pass (`brain_tests`).
- [x] Security suite passes (`test_auth.py`).
- [x] Build succeeds on `main` branch.
- [x] Version number updated in `CMakeLists.txt`.
- [x] `CHANGELOG.md` reflects all new changes.
- [x] `.env.example` includes any new configuration variables.
- [x] Docker image builds successfully.

## Tagging & Artifacts
- [x] Git tag created (`v1.0.0`).
- [x] Release notes generated (`plans/Release/RELEASE_NOTES_V1.0.0.md`).
- [x] `RELEASE_STATE.md` updated.
