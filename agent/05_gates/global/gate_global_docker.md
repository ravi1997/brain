# Gate: Global Docker

## Purpose

Ensures Docker containerization follows best practices for security, performance, and maintainability.

## Scope

Applies to any project with a `Dockerfile` or `docker-compose.yml`.

## Pass Criteria

### Build Success

- [ ] `docker build .` completes without errors
- [ ] Build time is reasonable (< 10 minutes for typical apps)
- [ ] Final image size is optimized (use multi-stage builds where applicable)

### Security

- [ ] Base images are pinned to specific versions (e.g., `node:20.11.0-alpine`, not `node:latest`)
- [ ] No secrets (API keys, passwords, tokens) are baked into the image
- [ ] Container runs as non-root user where possible
- [ ] Only necessary files are copied (`.dockerignore` is present and comprehensive)
- [ ] No unnecessary packages installed in final image

### Configuration

- [ ] Environment variables are used for configuration (not hardcoded values)
- [ ] Ports are properly exposed with `EXPOSE` directive
- [ ] Healthcheck is defined for services that support it
- [ ] Working directory is set appropriately

### Docker Compose (if present)

- [ ] `docker compose config` validates successfully
- [ ] Service dependencies are correctly defined
- [ ] Volume mounts are appropriate (code mounts for dev, named volumes for data)
- [ ] Network configuration is explicit when needed
- [ ] Environment files (`.env`) are documented but not committed with secrets

### Best Practices

- [ ] Multi-stage builds used to minimize final image size
- [ ] Build cache is leveraged (dependencies installed before code copy)
- [ ] Layer ordering optimized (least-changing layers first)
- [ ] Official or verified base images used

## Failure Remediation

### Build Fails

1. Read the error message carefully
2. Check for missing dependencies in package files
3. Verify base image is available and correct
4. Check network connectivity if pulling images fails

### Security Issues

1. Use `docker scan` or `trivy` to identify vulnerabilities
2. Pin base image versions in Dockerfile
3. Add `.dockerignore` to exclude sensitive files
4. Review Dockerfile for hardcoded secrets

### Image Too Large

1. Use Alpine-based images where possible
2. Implement multi-stage builds
3. Remove build dependencies in final stage
4. Use `.dockerignore` to exclude unnecessary files

## Related Files

- `11_rules/docker_rules.md`
- `06_skills/devops/skill_docker_baseline.md`
- `07_templates/devops/Dockerfile.template.md`
