# Docker Rules

## Purpose

Enforce best practices for Docker containerization across all projects.

## Core Principles

1. **Security First**: Never compromise security for convenience
2. **Minimal Images**: Keep images as small as possible
3. **Reproducibility**: Builds must be deterministic and repeatable
4. **Performance**: Optimize for build time and runtime efficiency

## Mandatory Rules

### Image Selection

- **MUST** use official or verified base images
- **MUST** pin base images to specific versions (never use `latest` tag)
- **SHOULD** prefer Alpine or slim variants when possible
- **MUST** document reason if using non-standard base image

### Security

- **MUST NOT** include secrets, API keys, or passwords in Dockerfile or image
- **MUST** run containers as non-root user in production
- **MUST** use `.dockerignore` to exclude sensitive files
- **MUST** keep base images updated to patch security vulnerabilities
- **SHOULD** scan images for vulnerabilities before deployment

### Build Optimization

- **MUST** use multi-stage builds for compiled languages
- **MUST** copy dependency files before source code (for layer caching)
- **MUST** combine related RUN commands to reduce layers
- **SHOULD** order Dockerfile instructions from least to most frequently changing
- **MUST** use `--no-cache-dir` flag for package managers (pip, npm)

### Configuration

- **MUST** use environment variables for configuration
- **MUST** expose ports explicitly with `EXPOSE` directive
- **MUST** define `WORKDIR` explicitly
- **SHOULD** include `HEALTHCHECK` for services
- **MUST** use `CMD` or `ENTRYPOINT` appropriately

### Documentation

- **MUST** document build arguments and environment variables
- **MUST** include Docker usage in README.md
- **SHOULD** add comments in Dockerfile for complex operations
- **MUST** maintain `.env.example` for required environment variables

## Docker Compose Rules

### Service Configuration

- **MUST** pin image versions for all services
- **MUST** use named volumes for persistent data
- **MUST** define service dependencies with `depends_on`
- **SHOULD** use healthchecks for dependent services
- **MUST** use `.env` file for environment variables

### Development vs Production

- **MUST** use `docker-compose.override.yml` for local dev overrides
- **MUST NOT** commit `.env` files with secrets
- **SHOULD** provide separate compose files for production if needed
- **MUST** document differences between dev and prod configurations

### Networking

- **SHOULD** define explicit networks when multiple services exist
- **MUST** use service names as hostnames for inter-service communication
- **MUST NOT** use `network_mode: host` unless absolutely necessary

## Best Practices

### Layer Caching

```dockerfile
# Good: Dependencies installed first
COPY package.json package-lock.json ./
RUN npm ci
COPY . .

# Bad: All files copied first
COPY . .
RUN npm ci
```

### Multi-Stage Builds

```dockerfile
# Good: Separate builder and runtime
FROM node:20-alpine AS builder
WORKDIR /app
COPY package*.json ./
RUN npm ci
COPY . .
RUN npm run build

FROM node:20-alpine
COPY --from=builder /app/dist ./dist
CMD ["node", "dist/index.js"]

# Bad: Single stage with build dependencies in final image
FROM node:20-alpine
COPY . .
RUN npm ci && npm run build
CMD ["node", "dist/index.js"]
```

### Non-Root User

```dockerfile
# Good: Create and use non-root user
RUN addgroup -g 1000 appuser && \
    adduser -D -u 1000 -G appuser appuser
USER appuser

# Bad: Running as root
# (no USER directive)
```

### Environment Variables

```dockerfile
# Good: Use ENV for defaults, allow override
ENV NODE_ENV=production
ENV PORT=3000

# Bad: Hardcoded values
# (no ENV directives, values in CMD)
```

## Anti-Patterns to Avoid

### Never Do This

```dockerfile
# ❌ Using latest tag
FROM node:latest

# ❌ Installing unnecessary packages
RUN apt-get install -y vim nano curl wget git

# ❌ Copying everything without .dockerignore
COPY . .

# ❌ Running as root in production
# (no USER directive)

# ❌ Hardcoding secrets
ENV API_KEY=secret123

# ❌ Not cleaning up in same layer
RUN apt-get update
RUN apt-get install -y package
RUN apt-get clean
```

### Do This Instead

```dockerfile
# ✅ Pin specific version
FROM node:20.11.0-alpine

# ✅ Install only what's needed
RUN apk add --no-cache curl

# ✅ Use .dockerignore and selective COPY
COPY package*.json ./
COPY src/ ./src/

# ✅ Run as non-root
USER node

# ✅ Use environment variables
ENV API_KEY=${API_KEY}

# ✅ Clean up in same layer
RUN apt-get update && \
    apt-get install -y package && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
```

## Validation

Before committing Dockerfile:

1. Run `docker build .` successfully
2. Check image size with `docker images`
3. Scan for vulnerabilities with `docker scan` or `trivy`
4. Verify non-root user with `docker run --rm <image> whoami`
5. Test application starts correctly

## Related Files

- `agent/05_gates/global/gate_global_docker.md`
- `agent/06_skills/devops/skill_docker_baseline.md`
- `agent/06_skills/devops/skill_compose_local_dev.md`
