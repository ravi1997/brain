# Gate: Global Security

## Pass Criteria

- [ ] `0` secrets found in the current changeset.
- [ ] `0` high-severity CVEs in dependencies.
- [ ] No unencrypted sensitive data in logs.
- [ ] `.env` and `*.key` files added to `.gitignore`.

## Fail Criteria

- [ ] Detection of a hardcoded API key or password.
- [ ] Use of "Insecure" packages (e.g., `pickle` from untrusted sources).

## Remediation

1. Revoke any leaked credentials immediately.
2. Use `git filter-branch` or similar to scrub history.
3. Update `10_security/security_baseline.md`.

## Related Files

- `10_security/secrets_policy.md`
