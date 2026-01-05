# Conventions

## Code style
- Python: ruff (format + lint) preferred.
- Use type hints for new code.
- Keep functions small; prefer early returns.
- Logging: structured where possible; include request_id/trace_id.

## Project layout defaults
- `app/` or `<package>/` for server code
- `tests/` for pytest
- `nginx/` for proxy config
- `scripts/` for dev helpers

## Git
- Branches: `fix/<slug>` or `feat/<slug>`
- Commits: imperative, scoped, with context:
  - `fix(nginx): handle upstream timeout`
  - `feat(auth): add refresh token rotation`

## Security
- Never log:
  - Authorization, cookies, tokens, passwords
  - request bodies containing PHI/PII
- Prefer allowlists to denylists for inputs.
- Validate file paths; block traversal.
- Use parameterized SQL.

## Documentation artifacts
Use templates under `agent/artifacts/`:
- PR summary
- Incident report
- Postmortem
- Runbook
