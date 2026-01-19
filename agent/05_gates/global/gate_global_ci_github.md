# Gate: Global CI/CD (GitHub Actions)

## Purpose

Ensures continuous integration and deployment pipelines are properly configured, maintained, and effective.

## Scope

Applies to any project with `.github/workflows/` directory.

## Pass Criteria

### Workflow Configuration

- [ ] At least one CI workflow exists (e.g., `ci.yml`)
- [ ] Workflow syntax is valid (use `actionlint` or GitHub's validator)
- [ ] Workflows trigger on appropriate events (push, pull_request, etc.)
- [ ] Workflow names and job names are descriptive

### Required Jobs

- [ ] **Lint**: Code style and formatting checks run
- [ ] **Test**: Unit and integration tests execute
- [ ] **Build**: Application builds successfully (or Docker image builds)
- [ ] All jobs must pass for PR merge

### Best Practices

- [ ] Caching is used for dependencies (npm, pip, cargo, etc.)
- [ ] Matrix builds test multiple versions/platforms where appropriate
- [ ] Secrets are stored in GitHub Secrets, not hardcoded
- [ ] Workflow runs complete in reasonable time (< 15 minutes for typical apps)
- [ ] Failed jobs provide clear, actionable error messages

### Security

- [ ] No secrets or credentials in workflow files
- [ ] Third-party actions are pinned to specific commits or versions
- [ ] `GITHUB_TOKEN` permissions are minimal (read-only where possible)
- [ ] Dependency scanning runs (npm audit, pip-audit, etc.)

### Pull Request Integration

- [ ] Required status checks are configured (if branch protection exists)
- [ ] PR template exists (`.github/PULL_REQUEST_TEMPLATE.md`)
- [ ] Workflows run on PRs from forks (if public repo)

### Release Workflow (if applicable)

- [ ] Release workflow triggers on tag push or manual dispatch
- [ ] Artifacts are built and attached to releases
- [ ] Release notes are generated or templated
- [ ] Version numbers follow semantic versioning

## Failure Remediation

### Workflow Syntax Error

1. Run `actionlint` locally to identify issues
2. Check YAML indentation and structure
3. Verify action versions exist
4. Test workflow with `act` tool locally if possible

### Jobs Failing

1. Review job logs in GitHub Actions tab
2. Reproduce failure locally using same commands
3. Check for environment-specific issues (paths, permissions)
4. Ensure all required secrets are configured

### Slow Workflows

1. Implement caching for dependencies
2. Parallelize independent jobs
3. Use matrix strategy to distribute work
4. Consider self-hosted runners for large projects

### Security Warnings

1. Pin third-party actions to commit SHAs
2. Move secrets to GitHub Secrets
3. Minimize token permissions
4. Enable Dependabot for action updates

## Related Files

- `11_rules/github_rules.md`
- `06_skills/devops/skill_github_actions_ci.md`
- `07_templates/devops/github_actions_ci.template.yml.md`
