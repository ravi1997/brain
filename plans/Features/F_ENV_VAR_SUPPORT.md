# Feature Spec: Environment Variable Support (C++)

## Metadata
- **ID**: STAB-02
- **Priority**: Critical
- **Status**: DRAFT

## Problem
Database credentials (host, port, user, password) are currently hardcoded in `src/postgres_storage.cpp` or passed via raw strings in `main.cpp`. This exposes secrets in version control and makes deployment brittle.

## Requirements
1. The system shall read `DB_HOST`, `DB_PORT`, `DB_USER`, `DB_PASS`, and `DB_NAME` from environment variables.
2. If variables are missing, provide sensible defaults (likely pointing to `localhost` for dev).
3. The `BrainServer` shall also support `SERVER_PORT` via environment variables.

## Technical Plan
- Implement `ConfigLoader` utility or update `PostgresStorage` constructor.
- Use `std::getenv` to fetch values.
- Update `docker-compose.yml` and `.github/workflows/ci.yml` to pass these variables from the host/runner.

## Verification
- Run the application without variables -> Should default to localhost.
- Run with `DB_HOST=127.0.0.1` -> Should log connection to 127.0.0.1.
- Verify `ci.yml` passes with the new structure.
