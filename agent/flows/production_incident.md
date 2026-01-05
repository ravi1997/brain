# Production Incident Flow (Read-only + Safe)

1. Stabilize:
   - identify impact, isolate failing components
   - reduce load if necessary (rate limit / maintenance mode)
2. Diagnose:
   - gather logs (redacted), error rates, recent deploys
3. Mitigate:
   - rollback to last known good if available
   - temporary config mitigation
4. Correct:
   - prepare PR and staging validation
5. Document:
   - incident report + postmortem
