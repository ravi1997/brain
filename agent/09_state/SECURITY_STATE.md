# Security State

## Assets Protected

- Source Code
- Brain Synaptic Weights (state/reflex_weights.json)
- Long-Term Memory (PostgreSQL)

## Known Vulnerabilities

- [LOW] Container running as root (FIXED: 2026-01-19)
- [HIGH] Hardcoded DB passwords in docker-compose.yml and ci.yml
- [MED] Unauthenticated admin port (9009)

## Audit History

- 2026-01-19 | Initial Repo Audit | DEGRADED (Root user found, secrets exposed)
- 2026-01-19 | Post-Fix Scan | IMPROVED (Rootless Docker implemented)

## Secret Rotation Schedule

- N/A
