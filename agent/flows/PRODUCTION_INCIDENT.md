# Production Incident Flow (Read-only + Safe)

1. Apply **policy/PRODUCTION_POLICY.md**
2. Collect evidence (sanitized)
3. Provide ranked hypotheses
4. Create staging reproduction plan
5. Prepare patch + tests in staging/dev
6. Provide rollout plan:
   - canary / low-risk steps
   - verification signals
   - rollback commands
