# Workflow: Security Review

## Purpose

Ensure the project is safe for real-world use.

## Inputs

- Codebase.

## Procedure

1. **Adopt Profile**: `profile_security_auditor.md`.
2. **Secret Scan**: Grep for API keys, passwords, and `.env` files in Git history and current files.
3. **Dependency Check**: Run `npm audit`, `pip-audit`, or `safety`.
4. **SARIF Scan**: (If available) Run static analysis tools.
5. **Policy Check**: Verify against `10_security/secrets_policy.md`.
6. **Gate**: Verify `05_gates/global/gate_global_security.md`.

## STOP Condition

- No "Critical" or "High" vulnerabilities remain.

## Related Files

- `10_security/security_baseline.md`
