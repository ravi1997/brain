# Release Plan - Brain Replica v0.2.0

## Release Strategy
The next release (v0.2.0) focuses on production-readiness and stabilization.

## Release Readiness Checklist
- [x] Non-root Docker execution.
- [ ] Environment variable support for secrets.
- [ ] Admin port authentication.
- [ ] Successful CI run on GitHub.
- [ ] Documentation for multi-port protocol.

## Release Artifacts
- Docker Image: `brain-replica:v0.2.0`
- Source Bundle
- Change Log

## Known Issues
- Local C++ build requires manual installation of `libpq-dev`.
- No TLS support for WebSocket proxy yet.
