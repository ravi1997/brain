# GitHub Actions Examples - Complete CI/CD Workflows

## Purpose

Production-ready GitHub Actions workflows for multiple stacks. All examples follow Agent OS `gate_global_ci_github.md` requirements.

## Agent OS Integration

**Satisfies Gates:**

- `agent/05_gates/global/gate_global_ci_github.md` ✅
- `agent/05_gates/global/gate_global_release.md` ✅

**Uses Skills:**

- `agent/06_skills/devops/skill_github_actions_ci.md`
- `agent/06_skills/devops/skill_github_release_flow.md`

**Follows Rules:**

- `agent/11_rules/github_rules.md`

---

## Table of Contents

1. [Branching Model](#branching-model)
2. [PR Template](#pr-template)
3. [Python CI](#python-ci-fastapi--flask)
4. [Node.js CI](#nodejs-ci-express)
5. [Next.js CI](#nextjs-ci)
6. [Flutter CI](#flutter-ci)
7. [Java CI](#java-ci-spring-boot)
8. [C++ CI](#c-ci-cmake)
9. [Release Workflows](#release-workflows)
10. [Troubleshooting](#troubleshooting)

---

## Branching Model

### Recommended Strategy

```
main (protected)
  ├── develop (optional, for larger teams)
  ├── feature/add-user-auth
  ├── feature/update-dashboard
  ├── fix/login-bug
  ├── chore/update-deps
  └── docs/api-documentation
```

### Branch Protection Rules

Configure in GitHub Settings → Branches → Add rule:

- **Branch name pattern**: `main`
- ✅ Require pull request reviews before merging (1 approval)
- ✅ Require status checks to pass before merging
  - CI
  - lint
  - test
  - build
- ✅ Require branches to be up to date before merging
- ✅ Require conversation resolution before merging

### Workflow

```bash
# 1. Create feature branch
git checkout -b feature/my-feature

# 2. Make changes and commit
git add .
git commit -m "feat: add new feature"

# 3. Push to GitHub
git push origin feature/my-feature

# 4. Open PR on GitHub
# 5. CI runs automatically
# 6. Request review
# 7. Address feedback
# 8. Merge when approved and CI passes
```

---

## PR Template

Create `.github/PULL_REQUEST_TEMPLATE.md`:

```markdown
## Description

Brief description of changes

## Type of Change

- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Related Issues

Closes #

## Testing

- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing completed
- [ ] Docker build succeeds

## Checklist

- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings
- [ ] Tests added/updated
- [ ] CI passes
- [ ] No secrets committed

## Screenshots (if applicable)

## Additional Notes
```

---

## Python CI (FastAPI / Flask)

### .github/workflows/ci.yml

```yaml
name: CI

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main, develop]
  workflow_dispatch:

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

jobs:
  lint:
    name: Lint
    runs-on: ubuntu-latest
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Setup Python
        uses: actions/setup-python@v5
        with:
          python-version: '3.11'
          cache: 'pip'

      - name: Install linting tools
        run: |
          pip install ruff black isort mypy

      - name: Run ruff
        run: ruff check .

      - name: Check formatting with black
        run: black --check .

      - name: Check import sorting
        run: isort --check-only .

      - name: Type check with mypy
        run: mypy . --ignore-missing-imports
        continue-on-error: true

  test:
    name: Test
    runs-on: ubuntu-latest
    strategy:
      matrix:
        python-version: ['3.10', '3.11', '3.12']
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Setup Python ${{ matrix.python-version }}
        uses: actions/setup-python@v5
        with:
          python-version: ${{ matrix.python-version }}
          cache: 'pip'

      - name: Install dependencies
        run: |
          pip install -r requirements.txt
          pip install pytest pytest-cov pytest-asyncio

      - name: Run tests
        run: pytest --cov=. --cov-report=xml --cov-report=html

      - name: Upload coverage to Codecov
        if: matrix.python-version == '3.11'
        uses: codecov/codecov-action@v3
        with:
          files: ./coverage.xml
          fail_ci_if_error: false

  security:
    name: Security Scan
    runs-on: ubuntu-latest
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Setup Python
        uses: actions/setup-python@v5
        with:
          python-version: '3.11'

      - name: Install pip-audit
        run: pip install pip-audit

      - name: Run security scan
        run: pip-audit -r requirements.txt
        continue-on-error: true

  docker:
    name: Docker Build
    runs-on: ubuntu-latest
    needs: [lint, test]
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3

      - name: Build Docker image
        uses: docker/build-push-action@v5
        with:
          context: .
          push: false
          tags: ${{ github.repository }}:${{ github.sha }}
          cache-from: type=gha
          cache-to: type=gha,mode=max
          load: true

      - name: Test Docker image
        run: |
          docker run -d --name test-container -p 8000:8000 -e DATABASE_URL=sqlite:///./test.db ${{ github.repository }}:${{ github.sha }}
          sleep 5
          curl --fail http://localhost:8000/health || exit 1
          docker stop test-container
```

### Gate Validation

✅ **Required Jobs**: lint, test, security, docker  
✅ **Caching**: pip dependencies cached  
✅ **Matrix**: Tests on Python 3.10, 3.11, 3.12  
✅ **Security**: pip-audit runs  
✅ **Docker**: Build validated

---

## Node.js CI (Express)

### .github/workflows/ci.yml

```yaml
name: CI

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main, develop]

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

jobs:
  lint:
    name: Lint
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Node.js
        uses: actions/setup-node@v4
        with:
          node-version: '20'
          cache: 'npm'

      - name: Install dependencies
        run: npm ci

      - name: Run ESLint
        run: npm run lint

      - name: Check formatting
        run: npm run format:check
        continue-on-error: true

  test:
    name: Test
    runs-on: ubuntu-latest
    strategy:
      matrix:
        node-version: [18, 20, 22]
    steps:
      - uses: actions/checkout@v4

      - name: Setup Node.js ${{ matrix.node-version }}
        uses: actions/setup-node@v4
        with:
          node-version: ${{ matrix.node-version }}
          cache: 'npm'

      - name: Install dependencies
        run: npm ci

      - name: Run tests
        run: npm test -- --coverage

      - name: Upload coverage
        if: matrix.node-version == 20
        uses: codecov/codecov-action@v3
        with:
          files: ./coverage/coverage-final.json

  security:
    name: Security Scan
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Node.js
        uses: actions/setup-node@v4
        with:
          node-version: '20'
          cache: 'npm'

      - name: Install dependencies
        run: npm ci

      - name: Run npm audit
        run: npm audit --audit-level=moderate
        continue-on-error: true

  docker:
    name: Docker Build
    runs-on: ubuntu-latest
    needs: [lint, test]
    steps:
      - uses: actions/checkout@v4

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3

      - name: Build Docker image
        uses: docker/build-push-action@v5
        with:
          context: .
          push: false
          tags: ${{ github.repository }}:${{ github.sha }}
          cache-from: type=gha
          cache-to: type=gha,mode=max
```

---

## Next.js CI

### .github/workflows/ci.yml

```yaml
name: CI

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

jobs:
  lint:
    name: Lint
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Node.js
        uses: actions/setup-node@v4
        with:
          node-version: '20'
          cache: 'npm'

      - name: Install dependencies
        run: npm ci

      - name: Run Next.js lint
        run: npm run lint

  test:
    name: Test
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Node.js
        uses: actions/setup-node@v4
        with:
          node-version: '20'
          cache: 'npm'

      - name: Install dependencies
        run: npm ci

      - name: Run tests
        run: npm test -- --coverage

      - name: Upload coverage
        uses: codecov/codecov-action@v3
        with:
          files: ./coverage/coverage-final.json

  build:
    name: Build
    runs-on: ubuntu-latest
    needs: [lint, test]
    steps:
      - uses: actions/checkout@v4

      - name: Setup Node.js
        uses: actions/setup-node@v4
        with:
          node-version: '20'
          cache: 'npm'

      - name: Install dependencies
        run: npm ci

      - name: Build Next.js app
        run: npm run build

      - name: Upload build artifacts
        uses: actions/upload-artifact@v3
        with:
          name: next-build
          path: .next/
          retention-days: 7

  docker:
    name: Docker Build
    runs-on: ubuntu-latest
    needs: [build]
    steps:
      - uses: actions/checkout@v4

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3

      - name: Build Docker image
        uses: docker/build-push-action@v5
        with:
          context: .
          push: false
          tags: ${{ github.repository }}:${{ github.sha }}
          cache-from: type=gha
          cache-to: type=gha,mode=max
```

---

## Flutter CI

### .github/workflows/ci.yml

```yaml
name: CI

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  analyze:
    name: Analyze
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Flutter
        uses: subosito/flutter-action@v2
        with:
          flutter-version: '3.16.0'
          channel: 'stable'
          cache: true

      - name: Get dependencies
        run: flutter pub get

      - name: Analyze code
        run: flutter analyze

      - name: Check formatting
        run: dart format --set-exit-if-changed .

  test:
    name: Test
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Flutter
        uses: subosito/flutter-action@v2
        with:
          flutter-version: '3.16.0'
          channel: 'stable'
          cache: true

      - name: Get dependencies
        run: flutter pub get

      - name: Run tests
        run: flutter test --coverage

      - name: Upload coverage
        uses: codecov/codecov-action@v3
        with:
          files: ./coverage/lcov.info

  build-web:
    name: Build Web
    runs-on: ubuntu-latest
    needs: [analyze, test]
    steps:
      - uses: actions/checkout@v4

      - name: Setup Flutter
        uses: subosito/flutter-action@v2
        with:
          flutter-version: '3.16.0'
          channel: 'stable'
          cache: true

      - name: Get dependencies
        run: flutter pub get

      - name: Build web
        run: flutter build web --release

      - name: Upload web build
        uses: actions/upload-artifact@v3
        with:
          name: web-build
          path: build/web/

  build-android:
    name: Build Android
    runs-on: ubuntu-latest
    needs: [analyze, test]
    steps:
      - uses: actions/checkout@v4

      - name: Setup Flutter
        uses: subosito/flutter-action@v2
        with:
          flutter-version: '3.16.0'
          channel: 'stable'
          cache: true

      - name: Setup Java
        uses: actions/setup-java@v3
        with:
          distribution: 'zulu'
          java-version: '11'

      - name: Get dependencies
        run: flutter pub get

      - name: Build APK
        run: flutter build apk --release

      - name: Upload APK
        uses: actions/upload-artifact@v3
        with:
          name: android-apk
          path: build/app/outputs/flutter-apk/app-release.apk
```

---

## Java CI (Spring Boot)

### .github/workflows/ci.yml

```yaml
name: CI

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

jobs:
  lint:
    name: Lint
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Java
        uses: actions/setup-java@v3
        with:
          distribution: 'temurin'
          java-version: '17'
          cache: 'maven'

      - name: Run Checkstyle
        run: ./mvnw checkstyle:check
        continue-on-error: true

  test:
    name: Test
    runs-on: ubuntu-latest
    strategy:
      matrix:
        java-version: [17, 21]
    steps:
      - uses: actions/checkout@v4

      - name: Setup Java ${{ matrix.java-version }}
        uses: actions/setup-java@v3
        with:
          distribution: 'temurin'
          java-version: ${{ matrix.java-version }}
          cache: 'maven'

      - name: Run tests
        run: ./mvnw test

      - name: Generate coverage report
        run: ./mvnw jacoco:report

      - name: Upload coverage
        if: matrix.java-version == 17
        uses: codecov/codecov-action@v3
        with:
          files: ./target/site/jacoco/jacoco.xml

  build:
    name: Build
    runs-on: ubuntu-latest
    needs: [lint, test]
    steps:
      - uses: actions/checkout@v4

      - name: Setup Java
        uses: actions/setup-java@v3
        with:
          distribution: 'temurin'
          java-version: '17'
          cache: 'maven'

      - name: Build with Maven
        run: ./mvnw clean package -DskipTests

      - name: Upload JAR
        uses: actions/upload-artifact@v3
        with:
          name: spring-boot-jar
          path: target/*.jar

  docker:
    name: Docker Build
    runs-on: ubuntu-latest
    needs: [build]
    steps:
      - uses: actions/checkout@v4

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3

      - name: Build Docker image
        uses: docker/build-push-action@v5
        with:
          context: .
          push: false
          tags: ${{ github.repository }}:${{ github.sha }}
          cache-from: type=gha
          cache-to: type=gha,mode=max
```

---

## C++ CI (CMake)

### .github/workflows/ci.yml

```yaml
name: CI

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  lint:
    name: Lint
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install clang-format
        run: sudo apt-get update && sudo apt-get install -y clang-format

      - name: Check formatting
        run: find src include -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror

  build-and-test:
    name: Build and Test
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest]
        build-type: [Debug, Release]
    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies (Ubuntu)
        if: matrix.os == 'ubuntu-latest'
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake build-essential libboost-all-dev

      - name: Install dependencies (macOS)
        if: matrix.os == 'macos-latest'
        run: brew install cmake boost

      - name: Configure CMake
        run: cmake -B build -DCMAKE_BUILD_TYPE=${{ matrix.build-type }}

      - name: Build
        run: cmake --build build --parallel $(nproc)

      - name: Run tests
        run: cd build && ctest --output-on-failure

  docker:
    name: Docker Build
    runs-on: ubuntu-latest
    needs: [build-and-test]
    steps:
      - uses: actions/checkout@v4

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3

      - name: Build Docker image
        uses: docker/build-push-action@v5
        with:
          context: .
          push: false
          tags: ${{ github.repository }}:${{ github.sha }}
          cache-from: type=gha
          cache-to: type=gha,mode=max
```

---

## Release Workflows

### Semantic Versioning Strategy

```
v1.0.0 - Initial release
v1.1.0 - New features (backward compatible)
v1.1.1 - Bug fixes
v2.0.0 - Breaking changes
```

### .github/workflows/release.yml

```yaml
name: Release

on:
  push:
    tags:
      - 'v*.*.*'

permissions:
  contents: write
  packages: write

jobs:
  validate:
    name: Validate Release
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: Validate version format
        run: |
          VERSION=${{ github.ref_name }}
          if [[ ! $VERSION =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
            echo "Invalid version format: $VERSION"
            exit 1
          fi

  build:
    name: Build Release
    runs-on: ubuntu-latest
    needs: validate
    steps:
      - uses: actions/checkout@v4

      - name: Setup environment
        uses: actions/setup-node@v4  # or setup-python, setup-java, etc.
        with:
          node-version: '20'
          cache: 'npm'

      - name: Install dependencies
        run: npm ci

      - name: Run tests
        run: npm test

      - name: Build
        run: npm run build

      - name: Package artifacts
        run: |
          tar -czf dist.tar.gz dist/
          sha256sum dist.tar.gz > dist.tar.gz.sha256

      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: release-artifacts
          path: |
            dist.tar.gz
            dist.tar.gz.sha256

  docker:
    name: Publish Docker Image
    runs-on: ubuntu-latest
    needs: validate
    steps:
      - uses: actions/checkout@v4

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3

      - name: Login to GitHub Container Registry
        uses: docker/login-action@v3
        with:
          registry: ghcr.io
          username: ${{ github.actor }}
          password: ${{ secrets.GITHUB_TOKEN }}

      - name: Extract version
        id: version
        run: |
          VERSION=${GITHUB_REF#refs/tags/}
          echo "VERSION=${VERSION}" >> $GITHUB_OUTPUT
          echo "VERSION_NO_V=${VERSION#v}" >> $GITHUB_OUTPUT

      - name: Build and push
        uses: docker/build-push-action@v5
        with:
          context: .
          push: true
          tags: |
            ghcr.io/${{ github.repository }}:${{ steps.version.outputs.VERSION }}
            ghcr.io/${{ github.repository }}:${{ steps.version.outputs.VERSION_NO_V }}
            ghcr.io/${{ github.repository }}:latest
          cache-from: type=gha
          cache-to: type=gha,mode=max

  release:
    name: Create GitHub Release
    runs-on: ubuntu-latest
    needs: [build, docker]
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: Download artifacts
        uses: actions/download-artifact@v3
        with:
          name: release-artifacts

      - name: Extract changelog
        id: changelog
        run: |
          VERSION=${GITHUB_REF#refs/tags/v}
          if [ -f CHANGELOG.md ]; then
            sed -n "/## \[${VERSION}\]/,/## \[/p" CHANGELOG.md | sed '$d' > release_notes.md
          else
            echo "Release $VERSION" > release_notes.md
            echo "" >> release_notes.md
            echo "## Changes" >> release_notes.md
            git log $(git describe --tags --abbrev=0 HEAD^)..HEAD --pretty=format:"- %s" >> release_notes.md
          fi

      - name: Create GitHub Release
        uses: softprops/action-gh-release@v1
        with:
          body_path: release_notes.md
          files: |
            dist.tar.gz
            dist.tar.gz.sha256
          draft: false
          prerelease: false
```

### Release Process

```bash
# 1. Update version in package files
# package.json, setup.py, pom.xml, etc.

# 2. Update CHANGELOG.md
# Add new version section with changes

# 3. Commit changes
git add .
git commit -m "chore: bump version to v1.2.3"

# 4. Create tag
git tag -a v1.2.3 -m "Release v1.2.3"

# 5. Push tag
git push origin main --tags

# 6. GitHub Actions automatically:
#    - Runs tests
#    - Builds artifacts
#    - Publishes Docker image
#    - Creates GitHub Release
```

---

## Troubleshooting

### Workflow Not Triggering

**Problem**: Pushed code but workflow didn't run

**Solution**: Check trigger configuration

```yaml
on:
  push:
    branches: [main]  # Must match your branch name
  pull_request:
    branches: [main]
```

### Cache Not Working

**Problem**: Dependencies downloaded every time

**Solution**: Ensure cache key is correct

```yaml
- uses: actions/setup-node@v4
  with:
    node-version: '20'
    cache: 'npm'  # Must have package-lock.json
```

### Job Timeout

**Problem**: Job exceeds 6 hour limit

**Solution**: Add timeout and optimize

```yaml
jobs:
  test:
    runs-on: ubuntu-latest
    timeout-minutes: 30  # Fail fast
```

### Secrets Not Available

**Problem**: `${{ secrets.MY_SECRET }}` is empty

**Solution**: Add secret in GitHub Settings → Secrets → Actions

### Docker Build Fails in CI

**Problem**: Works locally but fails in CI

**Solution**: Use BuildKit and check architecture

```yaml
- name: Set up Docker Buildx
  uses: docker/setup-buildx-action@v3

- name: Build
  uses: docker/build-push-action@v5
  with:
    platforms: linux/amd64  # Specify platform
```

### Coverage Upload Fails

**Problem**: Codecov upload fails

**Solution**: Make coverage optional

```yaml
- name: Upload coverage
  uses: codecov/codecov-action@v3
  with:
    fail_ci_if_error: false  # Don't fail CI
  continue-on-error: true
```

---

## Gate Validation Checklist

Before merging PR, verify against `agent/05_gates/global/gate_global_ci_github.md`:

- [ ] CI workflow exists (`.github/workflows/ci.yml`)
- [ ] Lint job runs
- [ ] Test job runs
- [ ] Build job runs (or Docker build)
- [ ] Workflows trigger on push and PR
- [ ] Caching configured for dependencies
- [ ] No secrets in workflow files
- [ ] Third-party actions pinned to versions
- [ ] Status checks required for merge
- [ ] Workflow completes in < 15 minutes

---

**Next**: See `docker_github_end_to_end.md` for complete workflow
