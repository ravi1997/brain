# Docker Examples - Complete Stack Coverage

## Purpose

Production-ready Docker examples for multiple stacks. All examples follow Agent OS `gate_global_docker.md` requirements.

## Agent OS Integration

**Satisfies Gates:**

- `agent/05_gates/global/gate_global_docker.md` ✅
- Stack-specific gates in `agent/05_gates/by_stack/` ✅

**Uses Skills:**

- `agent/06_skills/devops/skill_docker_baseline.md`
- `agent/06_skills/devops/skill_compose_local_dev.md`

**Follows Rules:**

- `agent/11_rules/docker_rules.md`

---

## Table of Contents

1. [Python (FastAPI)](#python-fastapi)
2. [Python (Flask)](#python-flask)
3. [Node.js (Express)](#nodejs-express)
4. [Next.js](#nextjs)
5. [Flutter](#flutter)
6. [Java (Spring Boot)](#java-spring-boot)
7. [C++ (CMake)](#c-cmake)
8. [Common Troubleshooting](#common-troubleshooting)

---

## Python (FastAPI)

### Dockerfile

```dockerfile
# Multi-stage build for minimal image size
FROM python:3.11-slim AS builder

WORKDIR /app

# Install dependencies first (better caching)
COPY requirements.txt .
RUN pip install --no-cache-dir --user -r requirements.txt

# Copy application code
COPY . .

# Runtime stage
FROM python:3.11-slim

# Create non-root user
RUN useradd -m -u 1000 appuser

WORKDIR /app

# Copy dependencies and app from builder
COPY --from=builder --chown=appuser:appuser /root/.local /home/appuser/.local
COPY --from=builder --chown=appuser:appuser /app /app

# Add user's local bin to PATH
ENV PATH=/home/appuser/.local/bin:$PATH

# Expose port
EXPOSE 8000

# Healthcheck
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
  CMD python -c "import requests; requests.get('http://localhost:8000/health')" || exit 1

# Switch to non-root user
USER appuser

# Start application
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
```

### docker-compose.yml

```yaml
version: '3.8'

services:
  api:
    build:
      context: .
      dockerfile: Dockerfile
    ports:
      - "${API_PORT:-8000}:8000"
    volumes:
      # Hot-reload for development
      - ./app:/app/app:ro
    environment:
      - DATABASE_URL=postgresql://${DB_USER:-postgres}:${DB_PASSWORD:-postgres}@postgres:5432/${DB_NAME:-myapp}
      - REDIS_URL=redis://redis:6379/0
      - LOG_LEVEL=${LOG_LEVEL:-info}
    depends_on:
      postgres:
        condition: service_healthy
      redis:
        condition: service_healthy
    networks:
      - app-network
    restart: unless-stopped

  postgres:
    image: postgres:16-alpine
    environment:
      - POSTGRES_DB=${DB_NAME:-myapp}
      - POSTGRES_USER=${DB_USER:-postgres}
      - POSTGRES_PASSWORD=${DB_PASSWORD:-postgres}
    volumes:
      - postgres-data:/var/lib/postgresql/data
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U ${DB_USER:-postgres}"]
      interval: 5s
      timeout: 5s
      retries: 5
    networks:
      - app-network

  redis:
    image: redis:7-alpine
    volumes:
      - redis-data:/data
    healthcheck:
      test: ["CMD", "redis-cli", "ping"]
      interval: 5s
      timeout: 3s
      retries: 5
    networks:
      - app-network

volumes:
  postgres-data:
  redis-data:

networks:
  app-network:
    driver: bridge
```

### .env.example

```env
# Application
API_PORT=8000
LOG_LEVEL=info

# Database
DB_NAME=myapp
DB_USER=postgres
DB_PASSWORD=changeme_in_production

# Never commit .env file with real secrets!
```

### .dockerignore

```dockerignore
.git
.gitignore
.env
__pycache__
*.pyc
*.pyo
*.pyd
.Python
.pytest_cache
.coverage
htmlcov
venv
env
.venv
*.log
README.md
tests
```

### Commands

```bash
# Build image
docker build -t myapp-api .

# Run standalone
docker run -p 8000:8000 -e DATABASE_URL=sqlite:///./test.db myapp-api

# Run with compose
cp .env.example .env
docker compose up

# View logs
docker compose logs -f api

# Run tests in container
docker compose exec api pytest

# Shell access
docker compose exec api bash

# Stop all services
docker compose down

# Reset volumes (WARNING: deletes data)
docker compose down -v
```

### Gate Validation

✅ **Security:**

- Base image pinned to `python:3.11-slim`
- Runs as non-root user (`appuser`)
- No secrets in Dockerfile
- `.dockerignore` excludes sensitive files

✅ **Performance:**

- Multi-stage build reduces image size
- Dependencies installed before code (layer caching)
- `--no-cache-dir` prevents pip cache bloat

✅ **Configuration:**

- Environment variables for all config
- Healthcheck defined
- Proper port exposure

---

## Python (Flask)

### Dockerfile

```dockerfile
FROM python:3.11-slim AS builder

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir --user -r requirements.txt

COPY . .

FROM python:3.11-slim

RUN useradd -m -u 1000 appuser

WORKDIR /app

COPY --from=builder --chown=appuser:appuser /root/.local /home/appuser/.local
COPY --from=builder --chown=appuser:appuser /app /app

ENV PATH=/home/appuser/.local/bin:$PATH
ENV FLASK_APP=app.py

EXPOSE 5000

HEALTHCHECK --interval=30s --timeout=3s CMD curl --fail http://localhost:5000/health || exit 1

USER appuser

CMD ["flask", "run", "--host=0.0.0.0"]
```

### docker-compose.yml

```yaml
version: '3.8'

services:
  web:
    build: .
    ports:
      - "${FLASK_PORT:-5000}:5000"
    volumes:
      - .:/app:ro
    environment:
      - FLASK_ENV=${FLASK_ENV:-development}
      - DATABASE_URL=postgresql://${DB_USER:-postgres}:${DB_PASSWORD:-postgres}@postgres:5432/${DB_NAME:-flask_app}
    depends_on:
      postgres:
        condition: service_healthy
    networks:
      - app-network

  postgres:
    image: postgres:16-alpine
    environment:
      - POSTGRES_DB=${DB_NAME:-flask_app}
      - POSTGRES_USER=${DB_USER:-postgres}
      - POSTGRES_PASSWORD=${DB_PASSWORD:-postgres}
    volumes:
      - postgres-data:/var/lib/postgresql/data
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U ${DB_USER:-postgres}"]
      interval: 5s
      timeout: 5s
      retries: 5
    networks:
      - app-network

volumes:
  postgres-data:

networks:
  app-network:
```

### Commands

```bash
docker build -t flask-app .
docker compose up
docker compose exec web flask db upgrade  # Run migrations
docker compose logs -f web
```

---

## Node.js (Express)

### Dockerfile

```dockerfile
FROM node:20-alpine AS builder

WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies
RUN npm ci --only=production

# Copy application code
COPY . .

# Runtime stage
FROM node:20-alpine

# Node user already exists in official image
WORKDIR /app

# Copy from builder
COPY --from=builder --chown=node:node /app /app

EXPOSE 3000

HEALTHCHECK --interval=30s --timeout=3s CMD wget --no-verbose --tries=1 --spider http://localhost:3000/health || exit 1

USER node

CMD ["node", "index.js"]
```

### docker-compose.yml

```yaml
version: '3.8'

services:
  app:
    build: .
    ports:
      - "${APP_PORT:-3000}:3000"
    volumes:
      - ./src:/app/src:ro
      - ./public:/app/public:ro
    environment:
      - NODE_ENV=${NODE_ENV:-development}
      - PORT=3000
      - DB_HOST=postgres
      - DB_PORT=5432
      - DB_NAME=${DB_NAME:-express_app}
      - DB_USER=${DB_USER:-postgres}
      - DB_PASSWORD=${DB_PASSWORD:-postgres}
      - REDIS_HOST=redis
      - REDIS_PORT=6379
    depends_on:
      postgres:
        condition: service_healthy
      redis:
        condition: service_healthy
    networks:
      - app-network
    restart: unless-stopped

  postgres:
    image: postgres:16-alpine
    environment:
      - POSTGRES_DB=${DB_NAME:-express_app}
      - POSTGRES_USER=${DB_USER:-postgres}
      - POSTGRES_PASSWORD=${DB_PASSWORD:-postgres}
    volumes:
      - postgres-data:/var/lib/postgresql/data
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U ${DB_USER:-postgres}"]
      interval: 5s
      timeout: 5s
      retries: 5
    networks:
      - app-network

  redis:
    image: redis:7-alpine
    volumes:
      - redis-data:/data
    healthcheck:
      test: ["CMD", "redis-cli", "ping"]
      interval: 5s
      timeout: 3s
      retries: 5
    networks:
      - app-network

volumes:
  postgres-data:
  redis-data:

networks:
  app-network:
```

### .dockerignore

```dockerignore
node_modules
npm-debug.log
.env
.git
.gitignore
README.md
tests
coverage
.nyc_output
dist
build
```

### Commands

```bash
docker build -t express-app .
docker compose up
docker compose exec app npm test
docker compose exec app sh  # Alpine uses sh, not bash
```

---

## Next.js

### Dockerfile

```dockerfile
FROM node:20-alpine AS deps

WORKDIR /app

COPY package*.json ./
RUN npm ci

FROM node:20-alpine AS builder

WORKDIR /app

COPY --from=deps /app/node_modules ./node_modules
COPY . .

# Build Next.js app
RUN npm run build

FROM node:20-alpine AS runner

WORKDIR /app

ENV NODE_ENV=production

RUN addgroup -g 1001 -S nodejs && \
    adduser -S nextjs -u 1001

# Copy necessary files
COPY --from=builder /app/public ./public
COPY --from=builder --chown=nextjs:nodejs /app/.next/standalone ./
COPY --from=builder --chown=nextjs:nodejs /app/.next/static ./.next/static

EXPOSE 3000

ENV PORT=3000
ENV HOSTNAME="0.0.0.0"

USER nextjs

CMD ["node", "server.js"]
```

### next.config.js

```javascript
/** @type {import('next').NextConfig} */
const nextConfig = {
  output: 'standalone', // Required for Docker
  reactStrictMode: true,
}

module.exports = nextConfig
```

### docker-compose.yml

```yaml
version: '3.8'

services:
  web:
    build:
      context: .
      dockerfile: Dockerfile
    ports:
      - "${NEXT_PORT:-3000}:3000"
    environment:
      - DATABASE_URL=postgresql://${DB_USER:-postgres}:${DB_PASSWORD:-postgres}@postgres:5432/${DB_NAME:-nextjs_app}
      - NEXTAUTH_URL=http://localhost:3000
      - NEXTAUTH_SECRET=${NEXTAUTH_SECRET:-changeme_in_production}
    depends_on:
      postgres:
        condition: service_healthy
    networks:
      - app-network
    restart: unless-stopped

  postgres:
    image: postgres:16-alpine
    environment:
      - POSTGRES_DB=${DB_NAME:-nextjs_app}
      - POSTGRES_USER=${DB_USER:-postgres}
      - POSTGRES_PASSWORD=${DB_PASSWORD:-postgres}
    volumes:
      - postgres-data:/var/lib/postgresql/data
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U ${DB_USER:-postgres}"]
      interval: 5s
      timeout: 5s
      retries: 5
    networks:
      - app-network

volumes:
  postgres-data:

networks:
  app-network:
```

### Development Compose (docker-compose.dev.yml)

```yaml
version: '3.8'

services:
  web:
    build:
      context: .
      target: deps  # Use deps stage for dev
    command: npm run dev
    ports:
      - "3000:3000"
    volumes:
      - .:/app
      - /app/node_modules
      - /app/.next
    environment:
      - NODE_ENV=development
```

### Commands

```bash
# Production build
docker build -t nextjs-app .
docker run -p 3000:3000 nextjs-app

# Development with hot-reload
docker compose -f docker-compose.dev.yml up

# Production with compose
docker compose up
```

---

## Flutter

### Dockerfile (Build Pipeline)

```dockerfile
FROM ubuntu:22.04 AS builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    curl \
    git \
    unzip \
    xz-utils \
    zip \
    libglu1-mesa \
    && rm -rf /var/lib/apt/lists/*

# Install Flutter
ENV FLUTTER_VERSION=3.16.0
RUN git clone --depth 1 --branch ${FLUTTER_VERSION} https://github.com/flutter/flutter.git /flutter

ENV PATH="/flutter/bin:${PATH}"

# Pre-download Flutter dependencies
RUN flutter doctor
RUN flutter precache

WORKDIR /app

# Copy pubspec files
COPY pubspec.* ./

# Get dependencies
RUN flutter pub get

# Copy source code
COPY . .

# Build web app
RUN flutter build web --release

# Serve stage (using nginx)
FROM nginx:alpine

COPY --from=builder /app/build/web /usr/share/nginx/html

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
```

### Dockerfile (Android Build)

```dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    curl \
    git \
    unzip \
    xz-utils \
    zip \
    libglu1-mesa \
    openjdk-11-jdk \
    && rm -rf /var/lib/apt/lists/*

# Install Flutter
ENV FLUTTER_VERSION=3.16.0
RUN git clone --depth 1 --branch ${FLUTTER_VERSION} https://github.com/flutter/flutter.git /flutter

ENV PATH="/flutter/bin:${PATH}"

# Accept Android licenses
RUN yes | flutter doctor --android-licenses

WORKDIR /app

COPY pubspec.* ./
RUN flutter pub get

COPY . .

# Build APK
RUN flutter build apk --release

# Output will be in /app/build/app/outputs/flutter-apk/app-release.apk
```

### docker-compose.yml (Web Development)

```yaml
version: '3.8'

services:
  flutter-web:
    build:
      context: .
      dockerfile: Dockerfile
    ports:
      - "8080:80"
    restart: unless-stopped
```

### Commands

```bash
# Build web app
docker build -t flutter-web .
docker run -p 8080:80 flutter-web

# Build Android APK
docker build -f Dockerfile.android -t flutter-android .
docker create --name flutter-build flutter-android
docker cp flutter-build:/app/build/app/outputs/flutter-apk/app-release.apk ./
docker rm flutter-build

# Development (requires X11 forwarding for desktop)
# Not recommended - use local Flutter for development
```

---

## Java (Spring Boot)

### Dockerfile

```dockerfile
FROM eclipse-temurin:17-jdk-alpine AS builder

WORKDIR /app

# Copy Maven/Gradle files
COPY pom.xml .
COPY mvnw .
COPY .mvn .mvn

# Download dependencies (cached layer)
RUN ./mvnw dependency:go-offline

# Copy source code
COPY src ./src

# Build application
RUN ./mvnw clean package -DskipTests

# Runtime stage
FROM eclipse-temurin:17-jre-alpine

RUN addgroup -g 1000 spring && \
    adduser -D -u 1000 -G spring spring

WORKDIR /app

# Copy JAR from builder
COPY --from=builder --chown=spring:spring /app/target/*.jar app.jar

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=3s CMD wget --no-verbose --tries=1 --spider http://localhost:8080/actuator/health || exit 1

USER spring

ENTRYPOINT ["java", "-jar", "app.jar"]
```

### docker-compose.yml

```yaml
version: '3.8'

services:
  app:
    build: .
    ports:
      - "${APP_PORT:-8080}:8080"
    environment:
      - SPRING_PROFILES_ACTIVE=${SPRING_PROFILE:-dev}
      - SPRING_DATASOURCE_URL=jdbc:postgresql://postgres:5432/${DB_NAME:-springapp}
      - SPRING_DATASOURCE_USERNAME=${DB_USER:-postgres}
      - SPRING_DATASOURCE_PASSWORD=${DB_PASSWORD:-postgres}
      - SPRING_REDIS_HOST=redis
      - SPRING_REDIS_PORT=6379
    depends_on:
      postgres:
        condition: service_healthy
      redis:
        condition: service_healthy
    networks:
      - app-network
    restart: unless-stopped

  postgres:
    image: postgres:16-alpine
    environment:
      - POSTGRES_DB=${DB_NAME:-springapp}
      - POSTGRES_USER=${DB_USER:-postgres}
      - POSTGRES_PASSWORD=${DB_PASSWORD:-postgres}
    volumes:
      - postgres-data:/var/lib/postgresql/data
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U ${DB_USER:-postgres}"]
      interval: 5s
      timeout: 5s
      retries: 5
    networks:
      - app-network

  redis:
    image: redis:7-alpine
    volumes:
      - redis-data:/data
    healthcheck:
      test: ["CMD", "redis-cli", "ping"]
      interval: 5s
      timeout: 3s
      retries: 5
    networks:
      - app-network

volumes:
  postgres-data:
  redis-data:

networks:
  app-network:
```

### .dockerignore

```dockerignore
target
.mvn/wrapper/maven-wrapper.jar
.git
.gitignore
README.md
*.log
```

### Commands

```bash
docker build -t spring-app .
docker compose up
docker compose exec app sh
docker compose logs -f app
```

---

## C++ (CMake)

### Dockerfile

```dockerfile
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy CMake files
COPY CMakeLists.txt .
COPY cmake ./cmake

# Copy source code
COPY src ./src
COPY include ./include

# Build
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --parallel $(nproc)

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -u 1000 appuser

WORKDIR /app

# Copy binary from builder
COPY --from=builder --chown=appuser:appuser /app/build/myapp /app/myapp

EXPOSE 8080

USER appuser

CMD ["./myapp"]
```

### docker-compose.yml

```yaml
version: '3.8'

services:
  app:
    build: .
    ports:
      - "${APP_PORT:-8080}:8080"
    volumes:
      - ./config:/app/config:ro
    environment:
      - CONFIG_PATH=/app/config/app.conf
    networks:
      - app-network
    restart: unless-stopped

networks:
  app-network:
```

### .dockerignore

```dockerignore
build
.git
.gitignore
README.md
*.md
.vscode
.idea
```

### Commands

```bash
docker build -t cpp-app .
docker run -p 8080:8080 cpp-app
docker compose up
```

---

## Common Troubleshooting

### Build Fails

**Problem**: `ERROR [internal] load metadata for docker.io/library/node:20-alpine`

**Solution**: Network issue or Docker daemon not running

```bash
# Check Docker daemon
docker info

# Try pulling base image manually
docker pull node:20-alpine
```

**Problem**: `COPY failed: no source files were specified`

**Solution**: Check .dockerignore isn't excluding required files

```bash
# List what's being sent to build context
docker build --no-cache --progress=plain . 2>&1 | grep COPY
```

### Port Conflicts

**Problem**: `Error starting userland proxy: listen tcp4 0.0.0.0:3000: bind: address already in use`

**Solution**: Port already in use

```bash
# Find process using port
lsof -i :3000
# or
netstat -tulpn | grep 3000

# Kill process or use different port
docker run -p 3001:3000 myapp
```

### Volume Mount Issues (Node.js)

**Problem**: `node_modules` from host overwriting container's

**Solution**: Use anonymous volume for node_modules

```yaml
volumes:
  - .:/app
  - /app/node_modules  # Anonymous volume takes precedence
```

### Python Dependencies Mismatch

**Problem**: Dependencies work locally but fail in Docker

**Solution**: Ensure requirements.txt is up to date

```bash
# Regenerate requirements.txt
pip freeze > requirements.txt

# Or use pip-tools
pip-compile requirements.in
```

### Flutter Build Caching

**Problem**: Flutter builds take forever every time

**Solution**: Use BuildKit caching

```bash
# Enable BuildKit
export DOCKER_BUILDKIT=1

# Build with cache
docker build --build-arg BUILDKIT_INLINE_CACHE=1 -t flutter-app .
```

### Java Gradle/Maven Caching

**Problem**: Dependencies downloaded every build

**Solution**: Copy dependency files first

```dockerfile
# Copy pom.xml first
COPY pom.xml .
RUN ./mvnw dependency:go-offline

# Then copy source
COPY src ./src
```

### C++ Missing Libraries

**Problem**: `error while loading shared libraries: libboost_system.so.1.74.0`

**Solution**: Install runtime dependencies in runtime stage

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0
```

### Permission Denied

**Problem**: `Permission denied` when running container

**Solution**: Ensure files are owned by container user

```dockerfile
COPY --from=builder --chown=appuser:appuser /app /app
```

### Healthcheck Failing

**Problem**: Container marked unhealthy

**Solution**: Check healthcheck command works inside container

```bash
# Exec into container
docker exec -it <container-id> sh

# Run healthcheck command manually
curl http://localhost:8000/health
```

---

## Gate Validation Checklist

Before committing Docker files, verify against `agent/05_gates/global/gate_global_docker.md`:

- [ ] Base images pinned to specific versions
- [ ] Multi-stage build used (where applicable)
- [ ] Runs as non-root user
- [ ] No secrets in Dockerfile or image
- [ ] .dockerignore present and comprehensive
- [ ] Environment variables used for configuration
- [ ] Healthcheck defined (for services)
- [ ] Ports properly exposed
- [ ] Build succeeds: `docker build .`
- [ ] Image size reasonable for stack

---

**Next**: See `github_examples.md` for CI/CD workflows
