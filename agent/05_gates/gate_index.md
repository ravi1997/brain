# Gate Index

## Purpose

Central registry of all quality and safety checkpoints.

## Global Gates (`05_gates/global/`)

- `gate_global_docs.md`: Minimum doc requirements.
- `gate_global_quality.md`: Code quality and testing thresholds.
- `gate_global_security.md`: Safety and secret management.
- `gate_global_release.md`: Final production readiness.
- `gate_global_docker.md`: Docker containerization best practices.
- `gate_global_ci_github.md`: GitHub Actions CI/CD requirements.

## Stack Gates (`05_gates/by_stack/`)

- `python/gate_python_pep8.md`
- `node/gate_node_npm_audit.md`
- `react/gate_react_prop_types.md`

## Enforcement (`05_gates/enforcement/`)

- `gate_failure_playbook.md`: What to do when a gate fails.
- `gate_exception_policy.md`: Rules for bypassing a gate (rare).
- `gate_signoff_checklist.md`: The binary "Ready" signal.
