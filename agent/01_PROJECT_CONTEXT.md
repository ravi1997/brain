# Project Context (Fill Once)

**Purpose:** Configure project-specific settings for AI agent
**When to use:** Once per project, during initial setup
**Prerequisites:** AI folder copied to project
**Outputs:** Configured AUTO_CONTEXT for agent use

---

## CRITICAL: Use Auto-Setup First

**Recommended:** Let the agent auto-detect everything:

```
User: "Setup AI folder for this project"
```

Agent will:
1. Detect project type (Python/C++/Java/etc.)
2. Find build system (CMake/Maven/npm/etc.)
3. Identify framework (Flask/Spring/React/etc.)
4. Fill ALL fields automatically
5. Report confidence level

**Manual setup only if auto-detection fails.**

---

## AUTO_CONTEXT (Universal Schema)

Copy/paste and edit. **Leave unknowns blank** - agent will infer.

```yaml
# CORE (Required)
app_name: "brain_replica" # REQUIRED
project_type: "cpp"       # python|nodejs|java|cpp|go|rust|flutter|static
PRIMARY_STACK: "cpp"      # OPTIONAL: Manual override for stack detection (e.g. "python", "java")
env: "dev"                # REQUIRED - dev|staging|production

# STRUCTURE
repo_root: "."
source_dir: "src/"        # src/|app/|lib/
build_dir: "build/"       # dist/|build/|target/
test_dir: "tests/"        # tests/|test/

# BUILD
build_system: "cmake"     # cmake|makefile|maven|npm|pip|uv|cargo
build_cmd: "cmake -B build && cmake --build build"
clean_cmd: "rm -rf build"

# PACKAGE MANAGER
package_manager: ""       # pip|uv|npm|maven|cargo|pub
install_cmd: ""

# RUNTIME
runtime: "gcc"            # python|node|java|gcc|go|dart
entrypoint: "build/brain_replica"
run_cmd: "./build/brain_replica"

# WEB (if applicable)
framework: ""             # flask|express|spring|react
server_type: ""           # gunicorn|node|tomcat
listen_host: "0.0.0.0"
app_port: 8000
health_path: "/healthz"

# DATABASE (if applicable)
db_kind: "postgres"       # postgres|mysql|sqlite|mongo|none
migration_tool: ""        # alembic|flyway|sequelize|none

# DOCKER (if applicable)
uses_docker: true
compose_file: "docker-compose.yml"
compose_backend_service: "brain"

# DEPLOYMENT (if applicable)
deployment_type: "docker" # docker|systemd|k8s|serverless
systemd_unit: ""

# TESTING
test_cmd: "ctest --test-dir build" # pytest|npm test|mvn test
lint_cmd: ""              # ruff check .|npm run lint

# SECURITY
has_phi_pii: false        # Default true for safety
```

See [`contracts/UNIVERSAL_PROJECT_SCHEMA.md`](contracts/UNIVERSAL_PROJECT_SCHEMA.md) for complete schema.

---

## Validation Checklist

Agent MUST verify:
- [ ] `app_name` is filled (REQUIRED)
- [ ] `project_type` is set (REQUIRED)
- [ ] `env` is correct (dev/staging/production)
- [ ] All blank fields processed by autofill
- [ ] Confidence level calculated
- [ ] If uncertain about env → defaulted to production

---

## See Also

- [`skills/project_auto_setup.md`](skills/project_auto_setup.md) - Auto-detection
- [`autofill/PATH_AND_SERVICE_INFERENCE.md`](autofill/PATH_AND_SERVICE_INFERENCE.md) - Inference rules
- [`examples/example_project_context.md`](examples/example_project_context.md) - Examples