# Detection: Stack Signals

## Purpose

Identifies specific languages, frameworks, and build tools using "fingerprints".

## Identification Logic

| Stack component | Signals (Files/Strings) | Fingerprint File |
|---|---|---|
| **Python** | `requirements.txt`, `pyproject.toml`, `.py` | `stack_fingerprints/python_*.md` |
| **Node.js** | `package.json`, `node_modules` | `stack_fingerprints/node_express.md` |
| **React/Vite** | `vite.config.ts`, `React` imports | `stack_fingerprints/react_vite.md` |
| **Java** | `pom.xml`, `build.gradle`, `.java` | `stack_fingerprints/java_spring.md` |
| **C++** | `CMakeLists.txt`, `.cpp`, `.hpp` | `stack_fingerprints/cpp_cmake.md` |
| **Docker** | `Dockerfile`, `docker-compose.yml`, `.dockerignore` | See stack-specific fingerprints |
| **Docker Compose** | `docker-compose.yml`, `docker-compose.override.yml`, `compose.yaml` | See stack-specific fingerprints |
| **DevContainer** | `.devcontainer/devcontainer.json`, `.devcontainer.json` | See stack-specific fingerprints |
| **Container Registry** | `ghcr.io` in configs, Docker Hub references | N/A |
| **GitHub Actions** | `.github/workflows/*.yml`, `.github/workflows/*.yaml` | N/A |
| **GitHub Templates** | `.github/PULL_REQUEST_TEMPLATE.md`, `.github/ISSUE_TEMPLATE/` | N/A |
| **GitHub Governance** | `.github/CODEOWNERS`, branch protection indicators | N/A |

## Procedure

1. **File Check**: Look for filenames in the root.
2. **Content Check**: If filenames are missing, grep for keywords in common entry files.
3. **Version Check**: Determine tool versions (e.g., `python --version`).
4. **Update State**: Write the detected stack to `agent/09_state/STACK_STATE.md`.

## Failure modes

- If conflicting signals are found (e.g. `pom.xml` AND `package.json`), identify if it's a monorepo.

## Related Files

- `stack_fingerprints/`
