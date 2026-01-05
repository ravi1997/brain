# Routing Rules (Keywords → Workflows)

**Purpose:** Map user requests to appropriate workflows based on keywords
**When to use:** When routing requests in `00_INDEX.md`
**Priority:** Match from top to bottom, first match wins

---

## How It Works

1. **Extract keywords** from user request
2. **Match against patterns** below (top to bottom)
3. **Route to workflow** or checklist
4. **If no match** → Use `flows/INCIDENT_TRIAGE.md`

---

## 🔴 Priority 1: Production Incidents (Immediate)

### HTTP Errors (502, 504, 5xx)
**Keywords:** `502`, `504`, `bad gateway`, `upstream`, `gateway timeout`, `5xx`

**Route:**
1. Run [`checklists/NGINX_502_EVIDENCE.md`](checklists/NGINX_502_EVIDENCE.md)
2. Follow [`workflows/nginx_502_504.md`](workflows/nginx_502_504.md)
3. Or use [`skills/nginx_gunicorn.md`](skills/nginx_gunicorn.md)

**Examples:**
- "Getting 502 errors"
- "Bad gateway timeout"
- "Upstream connection failed"

---

### Service Failures (systemd)
**Keywords:** `systemctl`, `journalctl`, `failed`, `inactive`, `dead`, `service down`

**Route:**
1. Run [`checklists/SYSTEMD_FAIL_EVIDENCE.md`](checklists/SYSTEMD_FAIL_EVIDENCE.md)
2. Follow [`workflows/systemd_failures.md`](workflows/systemd_failures.md)

**Examples:**
- "Service is dead"
- "systemctl shows failed"
- "App won't start"

---

### Database Issues
**Keywords:** `database`, `connection refused`, `too many connections`, `deadlock`, `constraint violation`

**Route:**
1. Run [`checklists/MIGRATION_FAIL_EVIDENCE.md`](checklists/MIGRATION_FAIL_EVIDENCE.md)
2. Follow [`workflows/db_migrations.md`](workflows/db_migrations.md)

**Examples:**
- "Database connection failed"
- "Too many connections"
- "Deadlock detected"

---

## 🟡 Priority 2: Build & Deploy Issues

### Docker Build Failures
**Keywords:** `docker build`, `compose`, `no space`, `permission denied`, `image`, `container`, `dockerfile`

**Route:**
1. Run [`checklists/DOCKER_BUILD_FAIL_EVIDENCE.md`](checklists/DOCKER_BUILD_FAIL_EVIDENCE.md)
2. Follow [`workflows/docker_dev_loop.md`](workflows/docker_dev_loop.md)

**Examples:**
- "Docker build failed"
- "No space left on device"
- "Permission denied in container"

---

### Migration Failures
**Keywords:** `alembic`, `migration`, `revision`, `downgrade`, `upgrade`, `schema`

**Route:**
1. Run [`checklists/MIGRATION_FAIL_EVIDENCE.md`](checklists/MIGRATION_FAIL_EVIDENCE.md)
2. Follow [`workflows/db_migrations.md`](workflows/db_migrations.md)

**Examples:**
- "Migration failed"
- "Alembic revision error"
- "Schema mismatch"

---

### Deployment Issues
**Keywords:** `deploy`, `deployment`, `rollout`, `release`, `production deploy`

**Route:**
1. Follow [`workflows/deploy_and_migrate.md`](workflows/deploy_and_migrate.md)

**Examples:**
- "Need to deploy to production"
- "Deployment failed"
- "Rollout stuck"

---

## 🟢 Priority 3: Performance Issues

### Slow Performance
**Keywords:** `slow`, `timeout`, `latency`, `performance`, `lag`, `hanging`, `cpu`, `memory`, `load`

**Route:**
1. Run [`checklists/PERF_REGRESSION_EVIDENCE.md`](checklists/PERF_REGRESSION_EVIDENCE.md)
2. Follow [`workflows/performance_profiling.md`](workflows/performance_profiling.md)

**Examples:**
- "App is slow"
- "High CPU usage"
- "Memory leak"
- "Request timeout"

---

## 🔒 Priority 4: Security Issues

### Security Incidents
**Keywords:** `sql injection`, `sqli`, `path traversal`, `attack`, `breach`, `vulnerability`, `exploit`, `unauthorized`

**Route:**
1. Follow [`workflows/security_incident.md`](workflows/security_incident.md)
2. Check [`workflows/security_sqli_path.md`](workflows/security_sqli_path.md)

**Examples:**
- "SQL injection detected"
- "Suspicious requests"
- "Unauthorized access"

---

## 🔵 Priority 5: Feature Development

### New Features
**Keywords:** `feature`, `implement`, `add`, `create`, `build`, `new functionality`

**Route:**
1. Follow [`workflows/feature_delivery.md`](workflows/feature_delivery.md)
2. Use [`flows/FEATURE_SPEC.md`](flows/FEATURE_SPEC.md)

**Examples:**
- "Add user authentication"
- "Implement payment system"
- "Create new API endpoint"

---

## 🟣 Priority 6: Maintenance & Operations

### Rollback
**Keywords:** `rollback`, `revert`, `undo`, `restore`, `previous version`

**Route:**
1. Follow [`workflows/rollback_recovery.md`](workflows/rollback_recovery.md)

**Examples:**
- "Rollback deployment"
- "Revert to previous version"

---

### Maintenance Mode
**Keywords:** `maintenance`, `downtime`, `scheduled`, `maintenance window`

**Route:**
1. Follow [`workflows/maintenance_mode.md`](workflows/maintenance_mode.md)

**Examples:**
- "Enable maintenance mode"
- "Schedule downtime"

---

## ⚪ Priority 7: General Debugging

### Test Failures
**Keywords:** `test`, `pytest`, `jest`, `junit`, `ctest`, `flutter test`, `failing test`, `test error`

**Route:**
1. **Python:** Use [`skills/pytest_debugging.md`](skills/pytest_debugging.md)
2. **Java/Junit:** Use [`workflows/java_dev_loop.md`](workflows/java_dev_loop.md)
3. **C++/CTest:** Use [`workflows/cpp_build_test.md`](workflows/cpp_build_test.md)
4. **Flutter:** Use [`workflows/flutter_dev_loop.md`](workflows/flutter_dev_loop.md)
5. **General:** Follow [`workflows/debug_basic.md`](workflows/debug_basic.md)

**Examples:**
- "Tests are failing"
- "JUnit showing errors"
- "CTest failed to run"
- "Flutter test failed"

---

### General Errors
**Keywords:** `error`, `exception`, `crash`, `bug`, `issue`, `problem`

**Route:**
1. Follow [`workflows/debug_basic.md`](workflows/debug_basic.md)
2. Or use [`flows/INCIDENT_TRIAGE.md`](flows/INCIDENT_TRIAGE.md)

**Examples:**
- "Getting an error"
- "App crashed"
- "Something's broken"

---

## 🎯 Special Cases

### Project Setup
**Keywords:** `setup`, `configure`, `initialize`, `new project`, `first time`

**Route:**
1. Use [`skills/project_auto_setup.md`](skills/project_auto_setup.md)
2. Read [`QUICKSTART.md`](QUICKSTART.md)

**Examples:**
- "Setup AI for this project"
- "Configure for new project"

---

### Code Review
**Keywords:** `review`, `check`, `audit`, `security review`, `code quality`

**Route:**
1. Follow [`workflows/feature_delivery.md`](workflows/feature_delivery.md) (review section)
2. Check [`gates/QUALITY_GATES.md`](gates/QUALITY_GATES.md)

**Examples:**
- "Review this code"
- "Security audit"

---

## 🔄 Fallback Rules

### If Uncertain
**No clear match?** → Use [`flows/INCIDENT_TRIAGE.md`](flows/INCIDENT_TRIAGE.md)

### If Multiple Matches
**Multiple keywords?** → Use highest priority match

### If Ambiguous
**User request unclear?** → Ask for clarification:
```
"I see you mentioned [KEYWORDS]. Are you experiencing:
1. [OPTION 1] → [WORKFLOW 1]
2. [OPTION 2] → [WORKFLOW 2]
3. [OPTION 3] → [WORKFLOW 3]

Which best describes your situation?"
```

---

## Routing Decision Matrix

| Request Type | Priority | Checklist | Workflow | Skill |
|--------------|----------|-----------|----------|-------|
| 502/504 errors | 🔴 P1 | NGINX_502 | nginx_502_504 | nginx_gunicorn |
| Service down | 🔴 P1 | SYSTEMD_FAIL | systemd_failures | - |
| DB issues | 🔴 P1 | MIGRATION_FAIL | db_migrations | alembic_migrations |
| Docker build | 🟡 P2 | DOCKER_BUILD | docker_dev_loop | docker_compose_debug |
| Slow performance | 🟢 P3 | PERF_REGRESSION | performance_profiling | - |
| Security | 🔒 P4 | - | security_incident | - |
| Feature | 🔵 P5 | - | feature_delivery | - |
| Rollback | 🟣 P6 | - | rollback_recovery | - |
| General debug | ⚪ P7 | - | debug_basic | - |

---

## Validation

Before routing, agent MUST:
- [ ] Extract keywords from request
- [ ] Match against patterns (top to bottom)
- [ ] State which route is being taken
- [ ] Confirm with user if ambiguous

---

## See Also

- [`00_INDEX.md`](00_INDEX.md) - Main router
- [`REFERENCE_MAP.md`](REFERENCE_MAP.md) - File reference
- [`TAXONOMY.md`](TAXONOMY.md) - Error categories
- [`flows/INCIDENT_TRIAGE.md`](flows/INCIDENT_TRIAGE.md) - Triage flow

