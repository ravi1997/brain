# Auto-Fix Loop (Default)

## Loop
1. Reproduce (test or script)
2. Fix smallest root cause
3. Run unit tests
4. Run lint/format
5. Run basic security checks (if touching inputs)
6. Commit + PR summary artifact

## Stop conditions
- Cannot reproduce after 2 attempts → request missing inputs (logs, config)
- Fix requires risky change → propose plan and safe mitigation first
