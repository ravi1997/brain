# Workflow: Docker Development Loop

**Purpose:** Debug Docker build and runtime issues
**When to use:** Docker build fails, containers won't start, or development loop broken
**Prerequisites:** Docker installed, docker-compose.yml exists
**Estimated time:** 15-30 minutes
**Outputs:** Working Docker environment

---

## Prerequisites

- [ ] Docker is installed and running
- [ ] docker-compose.yml exists
- [ ] You have disk space (check `df -h`)
- [ ] Environment detected

---

## Step 1: Diagnose

### Check Docker Status
// turbo
```bash
# Docker running?
docker ps

# Check logs
docker-compose logs

# Check disk space
docker system df
```

### Common Issues

**A) Build Failure**
```bash
# Check build output
docker-compose build --no-cache

# Common causes:
# - No space left
# - Network timeout
# - Missing dependency
```

**B) Container Won't Start**
```bash
# Check why
docker-compose up
# Look for:
# - Port already in use
# - Volume mount errors
# - Environment variable missing
```

**C) Permission Denied**
```bash
# Check file permissions
ls -la

# Fix ownership
sudo chown -R $USER:$USER .
```

---

## Step 2: Fix

### Clean Up (if needed)
```bash
# Remove old containers
docker-compose down

# Clean system
docker system prune -a

# Remove volumes (careful!)
docker volume prune
```

### Rebuild
```bash
# Build fresh
docker-compose build --no-cache

# Start
docker-compose up -d

# Check logs
docker-compose logs -f
```

---

## Step 3: Verify

// turbo
```bash
# All containers running?
docker-compose ps
# Expected: All "Up"

# Test application
curl http://localhost:8000/healthz
# Expected: 200 OK

# Check logs for errors
docker-compose logs | grep -i error
# Expected: No critical errors
```

---

## Completion Criteria

- ✅ All containers running
- ✅ Application accessible
- ✅ No errors in logs
- ✅ Can make code changes and see updates

---

## See Also

- [`../checklists/DOCKER_BUILD_FAIL_EVIDENCE.md`](../checklists/DOCKER_BUILD_FAIL_EVIDENCE.md)
- [`../skills/docker_compose_debug.md`](../skills/docker_compose_debug.md)
