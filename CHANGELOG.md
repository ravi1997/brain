# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-01-19

### Added
- Token-based authentication for all TCP server ports (`BRAIN_TOKEN` env var).
- GitHub Actions CI/CD workflow with build, test, lint (clang-format), and security scan (cppcheck).
- Environment variable support via `dnn::infra::Config`.
- Support for PostgreSQL and Redis as optional backends.
- `scripts/test_auth.py` for automated security verification.
- `.clang-format` configuration for code consistency.

### Fixed
- Fixed crash in `Brain::teach` caused by input size mismatch in `PlasticLayer`.
- Resolved linkage issues for `test_rl_engine` and other test targets.
- Fixed rootless Docker execution and permission issues in build directories.
- Improved brain startup time by moving initial vocabulary loading to a background thread.

### Changed
- Refactored `TcpServer` to track authenticated client sockets.
- Updated `BrainServer` to automatically apply security tokens to all regional servers.
- Updated `.env.example` with current configuration options.

## [0.1.0] - 2025-12-21
- Initial audit baseline.
- Basic Brain simulation loop with cortical regions.
- SQLite memory store implementation.
