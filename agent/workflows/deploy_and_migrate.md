# Workflow: Deploy and Migrate

**Purpose:** Deploy application with database migrations
**When to use:** Deploying new version to staging or production
**Prerequisites:** Code ready, tests passing, migrations tested
**Estimated time:** 30-60 minutes
**Outputs:** Deployed application, updated database

---

## Prerequisites

- [ ] All tests pass locally
- [ ] Code reviewed and approved
- [ ] Migrations tested in dev/staging
- [ ] Backup created (production)
- [ ] Rollback plan documented
- [ ] If production → `policy/PRODUCTION_POLICY.md`

---

## Step 1: Pre-Deployment Checks

### Verify Readiness
```bash
# Tests pass?
pytest -v

# Linting clean?
ruff check .

# Migrations valid?
alembic check

# No secrets in code?
grep -r "api_key\|password" . --exclude-dir=.git
```

### Create Backup (Production)
```bash
# Database backup
pg_dump mydb > backup_$(date +%Y%m%d_%H%M%S).sql

# Code backup
git tag release-$(date +%Y%m%d-%H%M%S)
git push --tags
```

---

## Step 2: Deploy Application

### For Docker
```bash
# Pull latest code
git pull origin main

# Build new image
docker-compose build

# Stop old containers
docker-compose down

# Start new containers
docker-compose up -d

# Check status
docker-compose ps
```

### For systemd
```bash
# Pull latest code
git pull origin main

# Install dependencies
pip install -r requirements.txt

# Restart service
sudo systemctl restart myapp
```

---

## Step 3: Run Migrations

### Apply Migrations
```bash
# Check current version
alembic current

# Upgrade database
alembic upgrade head

# Verify
alembic current
```

### If Migration Fails
```bash
# Rollback migration
alembic downgrade -1

# Fix issue
# ... debug ...

# Try again
alembic upgrade head
```

---

## Step 4: Verify Deployment

### Health Checks
```bash
# Application running?
curl -I https://myapp.com/healthz
# Expected: 200 OK

# Database connected?
curl https://myapp.com/api/health
# Expected: {"status": "ok", "database": "connected"}

# No errors in logs?
docker-compose logs --tail=50 | grep -i error
# or
sudo journalctl -u myapp -n 50 | grep -i error
```

### Smoke Tests
```bash
# Test critical endpoints
curl https://myapp.com/api/users
curl https://myapp.com/api/health

# Test UI (if applicable)
# Open browser and verify
```

---

## Step 5: Monitor

```bash
# Watch logs for 5-10 minutes
docker-compose logs -f
# or
sudo journalctl -u myapp -f

# Check metrics (if available)
# - Response times
# - Error rates
# - Resource usage
```

---

## Completion Criteria

- ✅ Application deployed successfully
- ✅ Migrations applied successfully
- ✅ Health checks passing
- ✅ No errors in logs
- ✅ Smoke tests pass
- ✅ Monitored for 5-10 minutes with no issues

---

## Rollback Plan

If deployment fails:

```bash
# 1. Rollback code
git revert HEAD
# or
git checkout <previous-tag>

# 2. Rollback database
alembic downgrade -1
# or restore from backup
psql mydb < backup_YYYYMMDD_HHMMSS.sql

# 3. Redeploy previous version
docker-compose up -d
# or
sudo systemctl restart myapp

# 4. Verify rollback
curl -I https://myapp.com/healthz
```

---

## Required Artifacts

- **Production:** `artifacts/incident_report.md` (if issues)
- **All:** Deployment notes in git tag or release notes

---

## Common Mistakes

❌ **Don't** deploy without backup (production)
❌ **Don't** run migrations before deploying code
❌ **Don't** skip health checks
❌ **Don't** deploy during peak hours (production)

---

## See Also

- [`../workflows/db_migrations.md`](db_migrations.md)
- [`../workflows/rollback_recovery.md`](rollback_recovery.md)
- [`../policy/PRODUCTION_POLICY.md`](../policy/PRODUCTION_POLICY.md)
