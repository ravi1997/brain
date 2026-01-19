# Feature Spec: Rootless Docker Execution

## Problem
Currently, the `Dockerfile` runs as `root`, which is a security risk.

## Solution
1. Create a `brain` user in the `Dockerfile`.
2. Ensure all data directories (`state/`, `data/`) are owned by this user.
3. Switch `USER` to `brain` before starting the application.

## Implementation Plan
- Update `Dockerfile` to add `useradd`.
- `chown` the `/app` directory.
- Update `docker-compose.yml` if needed (usually not if handled in Dockerfile).

## Verification
- `docker build -t brain-test .`
- `docker run --rm brain-test whoami` (should be `brain`)
- `docker run --rm brain-test ls -ld state` (should show `brain` ownership)
