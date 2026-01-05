# Workflow: Performance Profiling

**Purpose:** Diagnose and fix performance issues
**When to use:** App is slow, high latency, resource exhaustion
**Prerequisites:** Access to server, profiling tools
**Estimated time:** 30-60 minutes
**Outputs:** Performance improvements, profiling report

---

## Prerequisites

- [ ] Performance issue confirmed (baseline metrics)
- [ ] Access to application and database
- [ ] Profiling tools available
- [ ] Environment detected

---

## Step 1: Collect Baseline Metrics

### Application Metrics
```bash
# Response times
curl -w "@curl-format.txt" -o /dev/null -s https://myapp.com/api/endpoint

# Resource usage
top -b -n 1 | grep python
# or
docker stats --no-stream
```

### Database Metrics
```bash
# Slow queries
# PostgreSQL
SELECT * FROM pg_stat_statements ORDER BY mean_time DESC LIMIT 10;

# MySQL
SHOW FULL PROCESSLIST;
```

---

## Step 2: Profile the Application

### Python Profiling
```python
# Add to code temporarily
import cProfile
import pstats

profiler = cProfile.Profile()
profiler.enable()

# ... code to profile ...

profiler.disable()
stats = pstats.Stats(profiler)
stats.sort_stats('cumulative')
stats.print_stats(20)
```

### Or use py-spy
```bash
# Sample running process
py-spy top --pid <pid>

# Generate flamegraph
py-spy record -o profile.svg --pid <pid>
```

---

## Step 3: Identify Bottlenecks

### Common Issues

**A) N+1 Queries**
```python
# ❌ BAD (N+1 queries)
users = User.query.all()
for user in users:
    print(user.posts)  # Separate query for each user

# ✅ GOOD (1 query with join)
users = User.query.options(joinedload(User.posts)).all()
```

**B) Missing Index**
```sql
-- Check for sequential scans
EXPLAIN ANALYZE SELECT * FROM users WHERE email = 'test@example.com';

-- Add index if needed
CREATE INDEX idx_users_email ON users(email);
```

**C) Inefficient Algorithm**
```python
# ❌ BAD (O(n²))
for item in list1:
    if item in list2:  # list lookup is O(n)
        ...

# ✅ GOOD (O(n))
set2 = set(list2)  # Convert to set once
for item in list1:
    if item in set2:  # set lookup is O(1)
        ...
```

---

## Step 4: Apply Fixes

### Database Optimization
```bash
# Add indexes
# Optimize queries
# Use connection pooling
# Add caching
```

### Application Optimization
```python
# Cache expensive operations
from functools import lru_cache

@lru_cache(maxsize=128)
def expensive_function(arg):
    ...

# Use async for I/O
async def fetch_data():
    ...
```

---

## Step 5: Verify Improvements

```bash
# Re-run baseline tests
# Compare metrics

# Before: 500ms average
# After: 50ms average
# Improvement: 10x faster
```

---

## Completion Criteria

- ✅ Bottleneck identified
- ✅ Fix applied and tested
- ✅ Performance improved (measurable)
- ✅ No new issues introduced

---

## See Also

- [`../checklists/PERF_REGRESSION_EVIDENCE.md`](../checklists/PERF_REGRESSION_EVIDENCE.md)
- [`../workflows/performance.md`](performance.md)
