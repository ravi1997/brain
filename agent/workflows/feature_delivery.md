# Workflow: Feature Delivery

**Purpose:** End-to-end workflow for delivering new features
**When to use:** When implementing new functionality from spec to production
**Prerequisites:** Feature spec, access to codebase, test environment
**Estimated time:** Hours to days (depending on feature complexity)
**Outputs:** Working feature, tests, PR, documentation

---

## Prerequisites

Before starting, verify:
- [ ] Feature spec is clear and approved
- [ ] You have access to development environment
- [ ] You can run tests locally
- [ ] You understand the codebase structure
- [ ] You have reviewed `02_CONVENTIONS.md` for coding standards

**If any prerequisite is not met → STOP and resolve it first.**

---

## Step 1: Understand the Spec

Fill out the feature form to clarify requirements.

### Commands
```bash
# Copy the feature form template
cp agent/forms/FEATURE_MIN.md features/my-feature-spec.md

# Fill it out with:
# - User story
# - Acceptance criteria
# - API contracts (if any)
# - UI mockups (if any)
# - Data model changes
```

### Validation
- [ ] User story is clear ("As a [user], I want [goal], so that [benefit]")
- [ ] Acceptance criteria are testable
- [ ] Edge cases are identified
- [ ] Dependencies are listed

### If This Step Fails
- **Spec unclear?** → Ask user for clarification
- **Too complex?** → Break into smaller features
- **Dependencies missing?** → Resolve dependencies first

---

## Step 2: Design the Solution

Plan the implementation before coding.

### Design Checklist
- [ ] Identify files to modify
- [ ] Identify new files to create
- [ ] Plan database schema changes (if any)
- [ ] Plan API endpoints (if any)
- [ ] Identify potential risks

### For Non-Trivial Features
Create a decision record:
```bash
cp agent/artifacts/DECISION_RECORD.md docs/decisions/my-feature.md
# Document:
# - Problem statement
# - Considered alternatives
# - Chosen solution
# - Rationale
```

### Architecture Considerations
- [ ] Does this fit existing patterns?
- [ ] Will this scale?
- [ ] Are there security implications?
- [ ] Does this affect performance?

---

## Step 3: Create Implementation Plan

Break the work into small, reviewable commits.

### Example Plan
```markdown
## Implementation Steps

1. **Database Migration** (if needed)
   - Create migration file
   - Add new tables/columns
   - Test migration up/down

2. **Backend API**
   - Add route/endpoint
   - Add business logic
   - Add validation
   - Add error handling

3. **Frontend** (if applicable)
   - Add UI components
   - Add state management
   - Add API calls
   - Add error handling

4. **Tests**
   - Unit tests for business logic
   - Integration tests for API
   - E2E tests for critical flows

5. **Documentation**
   - Update API docs
   - Update user docs
   - Add code comments
```

### Commit Strategy
- Keep commits small (< 300 lines)
- One logical change per commit
- Write clear commit messages

---

## Step 4: Implement the Feature

Code the feature following the plan.

### Development Loop
```bash
# 1. Create feature branch
git checkout -b feat/my-feature

# 2. Make changes
# ... edit files ...

# 3. Run tests frequently
pytest tests/test_my_feature.py -v

# 4. Run linter
ruff check .
ruff format .

# 5. Commit when tests pass
git add .
git commit -m "feat(api): add user authentication endpoint"

# 6. Repeat for each step in plan
```

### Code Quality Checklist
- [ ] Follows conventions in `02_CONVENTIONS.md`
- [ ] Has type hints (Python) or types (TypeScript)
- [ ] Has docstrings/comments for complex logic
- [ ] No hardcoded values (use config/env vars)
- [ ] No secrets in code
- [ ] Error handling is comprehensive

---

## Step 5: Write Tests

Add comprehensive tests for the new feature.

### Test Strategy
Use `testing/TEST_STRATEGY.md` for guidance.

#### Unit Tests
```python
# tests/test_my_feature.py
def test_feature_happy_path():
    """Test the main success scenario"""
    result = my_feature(valid_input)
    assert result == expected_output

def test_feature_edge_cases():
    """Test edge cases"""
    result = my_feature(edge_case_input)
    assert result handles_edge_case_correctly

def test_feature_error_handling():
    """Test error scenarios"""
    with pytest.raises(ExpectedError):
        my_feature(invalid_input)
```

#### Integration Tests
```python
def test_api_endpoint(client):
    """Test API endpoint end-to-end"""
    response = client.post('/api/my-feature', json=test_data)
    assert response.status_code == 200
    assert response.json['result'] == expected
```

### Test Coverage
```bash
# Run tests with coverage
pytest --cov=app tests/

# Aim for >80% coverage on new code
```

---

## Step 6: Quality Gates

Ensure all quality checks pass before creating PR.

### Gate 1: Tests
// turbo
```bash
# All tests must pass
pytest -v
# Expected: All passed, 0 failed
```

### Gate 2: Linting
// turbo
```bash
# Code must be formatted
ruff format .

# No lint errors
ruff check .
# Expected: All checks passed
```

### Gate 3: Security
// turbo
```bash
# Check for secrets
grep -r "api_key\|password\|secret" . --exclude-dir=.git
# Expected: No matches (or only in .env.example)

# Check for PHI/PII in logs
grep -r "email\|phone\|ssn" app/logging/
# Expected: All redacted
```

### Gate 4: Type Checking (if applicable)
```bash
# Python
mypy app/

# TypeScript
npm run type-check
```

---

## Step 7: Manual Testing

Test the feature manually in development environment.

### Test Scenarios
- [ ] Happy path works as expected
- [ ] Edge cases are handled
- [ ] Error messages are clear
- [ ] UI is responsive (if applicable)
- [ ] No console errors (if web app)

### Test in Different Environments
```bash
# Test locally
python wsgi.py
# or
npm run dev

# Test in Docker (if used)
docker-compose up

# Test with production-like data (if safe)
```

---

## Step 8: Create Pull Request

Generate PR summary and create the pull request.

### Generate PR Summary
```bash
cp agent/artifacts/pr_summary.md .github/pr-my-feature.md
# Fill in:
# - What changed
# - Why it changed
# - How to test
# - Screenshots (if UI)
# - Breaking changes (if any)
```

### PR Checklist
- [ ] Title is clear and descriptive
- [ ] Description explains what and why
- [ ] Tests are included
- [ ] Documentation is updated
- [ ] No merge conflicts
- [ ] CI passes (if applicable)

### Create PR
```bash
# Push branch
git push origin feat/my-feature

# Create PR via GitHub/GitLab UI
# Or use CLI:
gh pr create --title "feat: add user authentication" --body-file .github/pr-my-feature.md
```

---

## Step 9: Code Review

Address review feedback.

### Review Process
1. Reviewer provides feedback
2. Address each comment
3. Make changes if needed
4. Push updates
5. Re-request review

### Responding to Feedback
```bash
# Make requested changes
# ... edit files ...

# Run tests again
pytest -v

# Commit changes
git add .
git commit -m "fix: address review feedback"

# Push updates
git push origin feat/my-feature
```

---

## Step 10: Merge and Deploy

After approval, merge and deploy the feature.

### Pre-Merge Checklist
- [ ] All review comments addressed
- [ ] All CI checks passing
- [ ] No merge conflicts
- [ ] Approved by required reviewers

### Merge
```bash
# Squash and merge (recommended)
# Or merge via UI

# Pull latest main
git checkout main
git pull origin main

# Delete feature branch
git branch -d feat/my-feature
```

### Deploy
Follow `deploy_and_migrate.md` for deployment process.

---

## Completion Criteria

This workflow is complete when:
- ✅ Feature is implemented and tested
- ✅ All tests pass (unit, integration, E2E)
- ✅ Code review is approved
- ✅ PR is merged to main
- ✅ Feature is deployed (if applicable)
- ✅ Feature works in production (if applicable)
- ✅ Documentation is updated

**Do NOT mark complete until all criteria are met.**

---

## Rollback Plan

If the feature causes issues after deployment:

```bash
# 1. Revert the merge commit
git revert -m 1 <merge-commit-hash>

# 2. Create hotfix PR
git checkout -b hotfix/revert-my-feature
git push origin hotfix/revert-my-feature

# 3. Deploy the revert
# Follow deploy_and_migrate.md
```

---

## Required Artifacts

- **Always:** `artifacts/pr_summary.md`
- **If complex:** `artifacts/DECISION_RECORD.md`
- **If database changes:** Migration files
- **If API changes:** Updated API documentation

---

## Common Mistakes to Avoid

❌ **Don't** start coding without understanding the spec
❌ **Don't** make commits too large (>300 lines)
❌ **Don't** skip tests - write them as you code
❌ **Don't** hardcode values - use configuration
❌ **Don't** commit secrets or API keys
❌ **Don't** merge without code review
❌ **Don't** deploy without testing

---

## Troubleshooting

### Issue: Tests failing locally
```bash
# Check test environment
pytest --version
python --version

# Run single test for debugging
pytest tests/test_my_feature.py::test_specific_case -v

# Check test database
# Make sure test DB is clean
```

### Issue: Merge conflicts
```bash
# Update your branch with latest main
git checkout main
git pull origin main
git checkout feat/my-feature
git rebase main

# Resolve conflicts
# ... edit files ...
git add .
git rebase --continue
```

### Issue: CI failing but tests pass locally
```bash
# Check CI logs for differences
# Common issues:
# - Different Python/Node version
# - Missing environment variables
# - Different dependencies
```

---

## See Also

- [`../forms/FEATURE_MIN.md`](../forms/FEATURE_MIN.md) - Feature spec template
- [`../testing/TEST_STRATEGY.md`](../testing/TEST_STRATEGY.md) - Testing guidance
- [`../gates/QUALITY_GATES.md`](../gates/QUALITY_GATES.md) - Quality requirements
- [`../workflows/deploy_and_migrate.md`](deploy_and_migrate.md) - Deployment workflow
- [`../02_CONVENTIONS.md`](../02_CONVENTIONS.md) - Coding conventions
