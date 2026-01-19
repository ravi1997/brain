# GitHub Rules

## Purpose

Establish best practices for GitHub repository management, branching strategy, pull requests, and CI/CD workflows.

## Core Principles

1. **Collaboration**: Make it easy for others to contribute
2. **Quality**: Maintain high code quality through automation
3. **Transparency**: Clear communication through PRs and issues
4. **Security**: Protect sensitive data and maintain secure workflows

## Branching Strategy

### Branch Naming

- **MUST** use descriptive branch names
- **SHOULD** follow pattern: `<type>/<description>`
  - `feature/add-user-auth`
  - `fix/login-bug`
  - `chore/update-deps`
  - `docs/api-documentation`

### Protected Branches

- **MUST** protect `main` or `master` branch
- **MUST** require pull request reviews before merging
- **SHOULD** require status checks to pass before merging
- **SHOULD** require branches to be up to date before merging
- **MAY** require signed commits for sensitive projects

### Branch Lifecycle

- **MUST** create feature branches from `main`
- **MUST** keep feature branches short-lived (< 1 week)
- **MUST** delete branches after merging
- **SHOULD** rebase or merge from `main` regularly to avoid conflicts

## Pull Requests

### PR Creation

- **MUST** use PR template if provided
- **MUST** write clear, descriptive PR title
- **MUST** include description of changes and motivation
- **SHOULD** reference related issues (e.g., "Closes #123")
- **MUST** ensure all CI checks pass before requesting review
- **SHOULD** keep PRs focused and reasonably sized (< 500 lines when possible)

### PR Review

- **MUST** have at least one approval before merging (for teams)
- **SHOULD** address all review comments or explain why not
- **MUST** resolve all conversations before merging
- **SHOULD** use "Request changes" for blocking issues, "Comment" for suggestions

### PR Merging

- **SHOULD** use "Squash and merge" for feature branches
- **MAY** use "Rebase and merge" for clean history
- **MUST** ensure commit message is clear after squash
- **MUST** delete branch after merge

## Commit Messages

### Format

**SHOULD** follow Conventional Commits:

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, no logic change)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks, dependency updates

### Examples

```
feat(auth): add JWT authentication

Implements JWT-based authentication for API endpoints.
Includes middleware for token validation and refresh.

Closes #45
```

```
fix(api): handle null response in user endpoint

Fixes crash when user data is not found.
```

## GitHub Actions / CI/CD

### Workflow Files

- **MUST** validate workflow syntax before committing
- **MUST** use descriptive workflow and job names
- **SHOULD** organize workflows logically (ci.yml, release.yml, etc.)
- **MUST** include comments for complex workflow logic

### Security

- **MUST NOT** commit secrets or credentials to workflows
- **MUST** use GitHub Secrets for sensitive data
- **MUST** pin third-party actions to commit SHA or version
- **SHOULD** minimize token permissions (use `permissions:` key)
- **SHOULD** enable Dependabot for action updates

### Performance

- **SHOULD** use caching for dependencies
- **SHOULD** parallelize independent jobs
- **SHOULD** set reasonable timeouts (timeout-minutes)
- **SHOULD** cancel in-progress runs for outdated commits

### Required Checks

- **MUST** run linting on all PRs
- **MUST** run tests on all PRs
- **SHOULD** run build verification on all PRs
- **MAY** run security scans on PRs

## Repository Configuration

### Required Files

- **MUST** have `README.md` with project description and setup instructions
- **SHOULD** have `CONTRIBUTING.md` with contribution guidelines
- **SHOULD** have `LICENSE` file
- **SHOULD** have `.github/PULL_REQUEST_TEMPLATE.md`
- **MAY** have `.github/ISSUE_TEMPLATE/` for structured issues

### CODEOWNERS

- **SHOULD** define CODEOWNERS for automatic review assignment
- **MUST** keep CODEOWNERS up to date with team changes
- **SHOULD** use team mentions rather than individual users when possible

### Security

- **MUST** enable Dependabot security updates
- **SHOULD** enable secret scanning
- **SHOULD** enable code scanning (CodeQL) for public repos
- **MUST NOT** commit `.env` files or secrets

## Release Management

### Versioning

- **MUST** follow Semantic Versioning (SemVer)
  - MAJOR.MINOR.PATCH (e.g., 1.2.3)
  - MAJOR: Breaking changes
  - MINOR: New features (backward compatible)
  - PATCH: Bug fixes (backward compatible)

### Tagging

- **MUST** create annotated tags for releases
- **MUST** prefix tags with `v` (e.g., `v1.2.3`)
- **SHOULD** include release notes in tag message

### GitHub Releases

- **MUST** create GitHub Release for each version
- **MUST** include changelog or release notes
- **SHOULD** attach build artifacts to releases
- **SHOULD** mark pre-releases appropriately

### Changelog

- **MUST** maintain `CHANGELOG.md`
- **SHOULD** follow Keep a Changelog format
- **MUST** update changelog before each release
- **MAY** automate changelog generation from commits

## Issue Management

### Issue Creation

- **SHOULD** use issue templates when provided
- **MUST** include clear description and reproduction steps for bugs
- **SHOULD** add appropriate labels
- **SHOULD** link related issues and PRs

### Issue Labels

**SHOULD** use consistent label scheme:

- `bug`: Something isn't working
- `enhancement`: New feature or request
- `documentation`: Documentation improvements
- `good first issue`: Good for newcomers
- `help wanted`: Extra attention needed
- `wontfix`: This will not be worked on

## Best Practices

### Repository Hygiene

- **MUST** keep repository clean (no committed build artifacts)
- **SHOULD** use `.gitignore` appropriately
- **SHOULD** keep dependencies up to date
- **MUST** remove stale branches regularly

### Documentation

- **MUST** document all environment variables
- **MUST** document setup and deployment procedures
- **SHOULD** include architecture diagrams for complex projects
- **SHOULD** keep documentation in sync with code

### Collaboration

- **SHOULD** respond to issues and PRs within 48 hours
- **SHOULD** be respectful and constructive in reviews
- **MUST** follow code of conduct (if present)

## Anti-Patterns to Avoid

### Never Do This

```yaml
# ❌ Hardcoded secrets in workflow
env:
  API_KEY: sk_live_abc123

# ❌ Using latest for actions
- uses: actions/checkout@latest

# ❌ No caching
- run: npm install

# ❌ Overly broad permissions
permissions: write-all
```

### Do This Instead

```yaml
# ✅ Use secrets
env:
  API_KEY: ${{ secrets.API_KEY }}

# ✅ Pin to version or SHA
- uses: actions/checkout@v4

# ✅ Use caching
- uses: actions/setup-node@v4
  with:
    cache: 'npm'
- run: npm ci

# ✅ Minimal permissions
permissions:
  contents: read
  pull-requests: write
```

## Validation Checklist

Before pushing to GitHub:

- [ ] All tests pass locally
- [ ] Linter passes
- [ ] Commit messages follow convention
- [ ] No secrets in code or config
- [ ] `.gitignore` is up to date
- [ ] Documentation updated if needed

Before merging PR:

- [ ] All CI checks pass
- [ ] Code reviewed and approved
- [ ] All conversations resolved
- [ ] Branch is up to date with main
- [ ] Commit message is clear

## Related Files

- `agent/05_gates/global/gate_global_ci_github.md`
- `agent/06_skills/devops/skill_github_actions_ci.md`
- `agent/06_skills/devops/skill_github_release_flow.md`
- `agent/04_workflows/09_pr_review_loop.md`
- `agent/04_workflows/10_release_process.md`
