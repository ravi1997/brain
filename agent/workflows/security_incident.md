# Workflow: Security Incident Response

**Purpose:** Handle security incidents and vulnerabilities
**When to use:** When security issue detected (injection, breach, vulnerability)
**Prerequisites:** Access to logs, security tools, incident response plan
**Estimated time:** 30-90 minutes (depending on severity)
**Outputs:** Incident contained, vulnerability fixed, incident report

---

## Prerequisites

- [ ] Security incident confirmed
- [ ] Incident severity assessed (P1-P4)
- [ ] Stakeholders notified (if P1/P2)
- [ ] Environment detected
- [ ] Production → `policy/PRODUCTION_POLICY.md`

---

## Step 1: Contain the Incident

### Immediate Actions (P1/P2)
```bash
# 1. Block malicious IP (if identified)
sudo iptables -A INPUT -s <malicious-ip> -j DROP

# 2. Disable affected endpoint (if applicable)
# Edit nginx config to return 503

# 3. Rotate compromised credentials
# Change API keys, passwords, tokens

# 4. Enable additional logging
# Increase log verbosity temporarily
```

---

## Step 2: Investigate

### Collect Evidence
```bash
# Check access logs
grep -E "sql|script|\.\./" /var/log/nginx/access.log

# Check application logs
grep -i "error\|exception" /var/log/myapp/app.log

# Check authentication logs
grep "Failed password" /var/log/auth.log

# Check for suspicious files
find /var/www -name "*.php" -mtime -1
```

### Identify Attack Vector
- SQL Injection?
- Path Traversal?
- XSS?
- Authentication bypass?
- DDoS?

---

## Step 3: Fix Vulnerability

### For SQL Injection
```python
# Bad (vulnerable)
query = f"SELECT * FROM users WHERE id = {user_id}"

# Good (safe)
query = "SELECT * FROM users WHERE id = %s"
cursor.execute(query, (user_id,))
```

### For Path Traversal
```python
# Bad (vulnerable)
file_path = f"/uploads/{filename}"

# Good (safe)
import os
filename = os.path.basename(filename)  # Remove path
file_path = os.path.join("/uploads", filename)
if not file_path.startswith("/uploads"):
    raise ValueError("Invalid path")
```

### For XSS
```python
# Use proper escaping
from markupsafe import escape
safe_input = escape(user_input)
```

---

## Step 4: Deploy Fix

```bash
# 1. Test fix locally
pytest tests/security/

# 2. Deploy to staging
# ... deploy ...

# 3. Verify fix
# ... test ...

# 4. Deploy to production
# Follow deploy_and_migrate.md
```

---

## Step 5: Verify & Monitor

```bash
# 1. Verify vulnerability is fixed
# Run security scanner

# 2. Monitor logs for 24 hours
tail -f /var/log/nginx/access.log | grep -E "sql|script"

# 3. Check for similar vulnerabilities
# Code review related endpoints
```

---

## Completion Criteria

- ✅ Incident contained
- ✅ Vulnerability identified and fixed
- ✅ Fix deployed and verified
- ✅ No ongoing malicious activity
- ✅ Incident report created
- ✅ Stakeholders notified

---

## Required Artifacts

- **Always:** `artifacts/incident_report.md`
- **If major:** `artifacts/postmortem.md`
- **Always:** Security patch notes

---

## See Also

- [`../workflows/security_sqli_path.md`](security_sqli_path.md)
- [`../policy/PRODUCTION_POLICY.md`](../policy/PRODUCTION_POLICY.md)
