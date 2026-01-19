# Project State

## Master Lifecycle State

**Current State**: STABLE / DEV  
**Stack**: C++ (CMake), Node.js (React/Vite)  
**Last Activity**: 2026-01-19  
**Health**: HEALTHY

## Recent Decisions

- [BOOT-01] | Use C++20 for core logic | Based on CMakeLists.txt.
- [BOOT-02] | React/Vite for frontend | Based on web/client/package.json.
- [STAB-01] | Rootless Docker Implemented | Container runs as non-root user.
- [STAB-02] | Env-based Config | DB/Redis credentials moved to environment.
- [STAB-03] | Optional Build | Postgres/Redis are no longer hard dependencies for build.

## Gaps & Blockers

- [ ] Implement Milestone 2 (M2: Access Control).

## Git Baseline

- **HEAD**: aa8c5407cc56da8581ff5ed6ee2074ef1367904f
- **Branch**: main
