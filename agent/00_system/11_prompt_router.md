# Prompt Router

## Purpose

This document defines the **safe routing algorithm** for selecting the optimal prompt file(s) from `prompts/` based on the user's situation and project state.

## Core Principles

1. **Safety First**: Never route to a prompt that conflicts with current state
2. **Single Primary**: Always select exactly ONE primary prompt
3. **Minimal Supporting**: Select 0-3 supporting prompts (avoid context overload)
4. **Stack Awareness**: Consider detected stack when routing DevOps/implementation prompts
5. **State Consistency**: Validate selected prompt is compatible with lifecycle phase

## Routing Algorithm

### Step 1: Analyze Input

**Extract from User Request**:

- **Intent Keywords**: new, existing, bug, fix, test, release, security, refactor, etc.
- **Urgency Markers**: emergency, critical, production, outage, down, etc.
- **Scope Indicators**: full project, single feature, documentation only, etc.
- **Technology Mentions**: Docker, Kubernetes, React, Python, etc.

**Extract from Project State**:

- Read `agent/09_state/PROJECT_STATE.md`
- Current lifecycle phase: IDLE, BOOTSTRAP, SPEC, PLAN, DEV, AUDIT, RELEASE
- Existing artifacts in `plans/`
- Detected stack from `agent/02_detection/`

### Step 2: Apply Decision Tree

```
START
  │
  ├─ URGENCY CHECK
  │   ├─ Contains "emergency" OR "production outage" OR "critical bug"?
  │   │   └─ YES → Route to: prompts/by_scenario/emergency_hotfix.txt
  │   │            STOP (no supporting prompts in emergency)
  │   └─ NO → Continue
  │
  ├─ PROJECT EXISTENCE CHECK
  │   ├─ PROJECT_STATE.md shows "IDLE" AND no code files detected?
  │   │   └─ YES → Route to: prompts/by_entrypoint/new_project.txt
  │   │            Continue to Step 3 (supporting prompts)
  │   └─ NO → Continue
  │
  ├─ ENTRYPOINT CHECK (for existing projects)
  │   ├─ User wants "only SRS" OR "only documentation"?
  │   │   └─ YES → Route to: prompts/by_entrypoint/srs_only.txt
  │   │
  │   ├─ User wants "only plan" OR "only backlog"?
  │   │   └─ YES → Route to: prompts/by_entrypoint/plan_only.txt
  │   │
  │   ├─ User wants "implement" OR "build feature"?
  │   │   └─ YES → Route to: prompts/by_entrypoint/implement_only.txt
  │   │
  │   ├─ User wants "test" OR "testing"?
  │   │   └─ YES → Route to: prompts/by_entrypoint/test_only.txt
  │   │
  │   ├─ User wants "security audit" OR "vulnerability scan"?
  │   │   └─ YES → Route to: prompts/by_entrypoint/security_audit_only.txt
  │   │
  │   ├─ User wants "PR review" OR "code review"?
  │   │   └─ YES → Route to: prompts/by_entrypoint/pr_review_only.txt
  │   │
  │   ├─ User wants "release" OR "deploy" OR "ship"?
  │   │   └─ YES → Route to: prompts/by_entrypoint/release_only.txt
  │   │
  │   └─ None of above → Continue to SCENARIO CHECK
  │
  ├─ SCENARIO CHECK (specific problems)
  │   ├─ User reports "bug" OR "error" OR "not working"?
  │   │   └─ YES → Route to: prompts/by_scenario/bug_fix.txt
  │   │
  │   ├─ User wants "refactor" OR "clean up code"?
  │   │   └─ YES → Route to: prompts/by_scenario/refactor.txt
  │   │
  │   ├─ User reports "slow" OR "performance issue"?
  │   │   └─ YES → Route to: prompts/by_scenario/performance_investigation.txt
  │   │
  │   ├─ User wants "update dependencies" OR "upgrade libraries"?
  │   │   └─ YES → Route to: prompts/by_scenario/dependency_update.txt
  │   │
  │   ├─ User wants "database migration" OR "schema change"?
  │   │   └─ YES → Route to: prompts/by_scenario/data_migration.txt
  │   │
  │   ├─ User mentions "legacy code" OR "old system"?
  │   │   └─ YES → Route to: prompts/by_scenario/legacy_integration.txt
  │   │
  │   ├─ User reports "flaky tests" OR "intermittent failures"?
  │   │   └─ YES → Route to: prompts/by_scenario/flaky_test.txt
  │   │
  │   ├─ User reports "circular import" OR "dependency cycle"?
  │   │   └─ YES → Route to: prompts/by_scenario/circular_dependency.txt
  │   │
  │   ├─ User reports "API failure" OR "external service down"?
  │   │   └─ YES → Route to: prompts/by_scenario/api_failure.txt
  │   │
  │   ├─ User has "conflicting requirements"?
  │   │   └─ YES → Route to: prompts/by_scenario/conflict_resolution.txt
  │   │
  │   ├─ User request is "vague" OR "unclear"?
  │   │   └─ YES → Route to: prompts/by_scenario/ambiguous_input.txt
  │   │
  │   ├─ User mentions "multiple environments" OR "config management"?
  │   │   └─ YES → Route to: prompts/by_scenario/multi_env_config.txt
  │   │
  │   └─ None of above → DEFAULT ROUTE
  │
  └─ DEFAULT ROUTE
      └─ Route to: prompts/by_entrypoint/existing_project.txt
```

### Step 3: Select Supporting Prompts

**Only after primary prompt is selected**, check for supporting prompts:

#### DevOps Supporting Prompts

**Condition**: User mentions Docker OR Kubernetes OR CI/CD OR deployment

**Detection**:

- Check for `Dockerfile` or `docker-compose.yml`
- Check for `.github/workflows/` or `.gitlab-ci.yml`
- User request contains: "docker", "container", "CI", "CD", "deploy"

**Route to** (select up to 2):

1. `prompts/examples_devops/docker_examples.md` (if Docker detected/mentioned)
2. `prompts/examples_devops/github_examples.md` (if GitHub Actions detected/mentioned)
3. `prompts/examples_devops/docker_github_end_to_end.md` (if both detected)

#### Security Supporting Prompts

**Condition**: Primary prompt is NOT already security-focused AND (user mentions security OR project handles sensitive data)

**Detection**:

- User request contains: "security", "vulnerability", "auth", "encryption", "GDPR", "compliance"
- Detected stack includes: web framework, API, database

**Route to**:

- `prompts/by_entrypoint/security_audit_only.txt`

#### Testing Supporting Prompts

**Condition**: Primary prompt is NOT already test-focused AND (user mentions testing OR project has low coverage)

**Detection**:

- User request contains: "test", "coverage", "quality"
- Detected test files < 10% of code files

**Route to**:

- `prompts/by_entrypoint/test_only.txt`

### Step 4: Validate Compatibility

**State Compatibility Check**:

| Primary Prompt | Compatible States | Incompatible States |
|----------------|-------------------|---------------------|
| `new_project.txt` | IDLE | SPEC, PLAN, DEV, AUDIT, RELEASE |
| `existing_project.txt` | ANY | None |
| `srs_only.txt` | IDLE, BOOTSTRAP, SPEC | DEV, AUDIT, RELEASE |
| `plan_only.txt` | SPEC, PLAN | IDLE, DEV, AUDIT, RELEASE |
| `implement_only.txt` | PLAN, DEV | IDLE, BOOTSTRAP, SPEC |
| `test_only.txt` | DEV, AUDIT | IDLE, BOOTSTRAP, SPEC |
| `security_audit_only.txt` | DEV, AUDIT | IDLE, BOOTSTRAP |
| `release_only.txt` | AUDIT, RELEASE | IDLE, BOOTSTRAP, SPEC, PLAN |
| `emergency_hotfix.txt` | ANY | None |

**Validation Rules**:

1. If selected prompt is incompatible with current state:
   - Check if state is stale (user may have made changes outside Agent OS)
   - If state is fresh: reject routing and suggest compatible prompt
   - If state is stale: warn user and offer to update state first

2. If supporting prompts conflict with primary:
   - Remove conflicting supporting prompts
   - Log warning in routing report

### Step 5: Generate Routing Report

**Output**: `plans/Orchestration/prompt_routing.md`

**Template**:

```markdown
# Prompt Routing Report

**Generated**: [ISO 8601 timestamp]
**Orchestration ID**: [unique ID]

## Input Analysis

### User Request
[Original user request text]

### Extracted Intent
- **Primary Intent**: [e.g., "Create new project"]
- **Urgency**: [Normal / High / Critical]
- **Scope**: [Full project / Single phase / Specific problem]
- **Technology Mentions**: [e.g., "Docker, React, PostgreSQL"]

### Project State
- **Current Phase**: [from PROJECT_STATE.md]
- **Existing Artifacts**: [list key files in plans/]
- **Detected Stack**: [from agent/02_detection/]

## Routing Decision

### Selected Primary Prompt
- **File**: `prompts/by_entrypoint/new_project.txt`
- **Reason**: User requested creation of new web application with no existing code
- **Compatibility**: ✅ Compatible with IDLE state

### Supporting Prompts
1. **File**: `prompts/examples_devops/docker_examples.md`
   - **Reason**: User specified Docker deployment requirement
   - **Compatibility**: ✅ Compatible with new_project.txt

2. **File**: `prompts/by_entrypoint/security_audit_only.txt`
   - **Reason**: Web application requires security baseline
   - **Compatibility**: ✅ Compatible with new_project.txt

### Rejected Prompts
- `prompts/by_scenario/emergency_hotfix.txt`: Not an emergency situation
- `prompts/by_entrypoint/existing_project.txt`: No existing codebase detected
- `prompts/examples_devops/github_examples.md`: GitHub Actions not mentioned or detected

## Validation Results

- ✅ Primary prompt compatible with current state
- ✅ No conflicts between primary and supporting prompts
- ✅ Context budget within limits (estimated 23K tokens)

## Next Steps

1. Load primary prompt: `prompts/by_entrypoint/new_project.txt`
2. Load supporting prompts (2 files)
3. Proceed to subtask decomposition (Phase 4)
```

## Safety Rules

### Rule 1: Never Auto-Add DevOps

**Problem**: Orchestrator might add Docker/GitHub prompts when not needed

**Rule**: Only route to DevOps prompts if:

- User explicitly mentions Docker/CI/CD, OR
- Corresponding files detected (Dockerfile, .github/workflows/), OR
- Primary prompt explicitly requires it (e.g., release_only.txt)

**Enforcement**: Step 3 checks for explicit triggers

### Rule 2: Emergency Isolation

**Problem**: Emergency prompts should not be diluted with supporting prompts

**Rule**: If routing to `emergency_hotfix.txt`:

- Do NOT add any supporting prompts
- Skip Step 3 entirely
- Minimize orchestration overhead

**Enforcement**: Step 2 URGENCY CHECK stops immediately after routing

### Rule 3: No Circular Routing

**Problem**: Prompt A might reference Prompt B which references Prompt A

**Rule**: Maximum routing depth = 1

- Primary prompt can reference workflows/templates
- Supporting prompts can reference workflows/templates
- Prompts CANNOT reference other prompts

**Enforcement**: Routing algorithm does not recursively load referenced prompts

### Rule 4: Context Budget Enforcement

**Problem**: Loading too many prompts exceeds token limits

**Rule**: Maximum supporting prompts = 3

**Estimated Token Costs**:

- Primary prompt: ~5K tokens
- Supporting prompt: ~3K tokens each
- Maximum total: 5K + (3 × 3K) = 14K tokens

**Enforcement**: Step 3 limits supporting prompts to 3

### Rule 5: State Staleness Detection

**Problem**: PROJECT_STATE.md might be outdated

**Staleness Indicators**:

- State file last modified > 7 days ago
- State shows IDLE but code files exist
- State shows DEV but no implementation files in plans/

**Action When Stale**:

1. Warn user: "Project state may be outdated"
2. Offer to run state recovery: `agent/09_state/RECOVERY_CHECKPOINT.md`
3. Wait for user confirmation before routing

**Enforcement**: Step 4 validation checks state freshness

## Routing Examples

### Example 1: New Web App with Docker

**User Request**: "Create a new React web app with Docker deployment"

**Routing**:

- **Primary**: `prompts/by_entrypoint/new_project.txt` (new project detected)
- **Supporting**:
  - `prompts/examples_devops/docker_examples.md` (Docker mentioned)
- **Rejected**:
  - `prompts/examples_devops/github_examples.md` (not mentioned)

### Example 2: Bug Fix in Existing Project

**User Request**: "Fix the login bug where users can't authenticate"

**Routing**:

- **Primary**: `prompts/by_scenario/bug_fix.txt` (bug reported)
- **Supporting**: None (bug fix is self-contained)
- **Rejected**:
  - `prompts/by_entrypoint/existing_project.txt` (too broad)

### Example 3: Production Outage

**User Request**: "URGENT: API is down, users can't access the site"

**Routing**:

- **Primary**: `prompts/by_scenario/emergency_hotfix.txt` (urgency detected)
- **Supporting**: None (emergency isolation rule)
- **Rejected**: All others (emergency takes priority)

### Example 4: Release Preparation

**User Request**: "Prepare for v2.0 release"

**Routing**:

- **Primary**: `prompts/by_entrypoint/release_only.txt` (release intent)
- **Supporting**:
  - `prompts/by_entrypoint/security_audit_only.txt` (release requires security check)
  - `prompts/by_entrypoint/test_only.txt` (release requires test validation)
- **Rejected**:
  - `prompts/by_entrypoint/implement_only.txt` (release assumes implementation done)

### Example 5: Ambiguous Request

**User Request**: "Make the app better"

**Routing**:

- **Primary**: `prompts/by_scenario/ambiguous_input.txt` (vague request)
- **Supporting**: None (need clarification first)
- **Rejected**: All specific prompts (intent unclear)

## Integration with Orchestration Protocol

The prompt router is invoked during **Phase 3: Prompt Selection** of the orchestration protocol.

**Inputs**:

- User request (from Phase 2: Situation Identification)
- Project state (from `agent/09_state/PROJECT_STATE.md`)
- Detected stack (from `agent/02_detection/`)

**Outputs**:

- `plans/Orchestration/prompt_routing.md` (routing report)
- List of selected prompt file paths (for Phase 4: Subtask Decomposition)

**Error Handling**:

- If no prompt matches: route to `prompts/by_scenario/ambiguous_input.txt`
- If state is stale: pause and request state recovery
- If multiple prompts equally valid: prefer by_entrypoint over by_scenario

## Routing Metadata

Every routing decision MUST record:

```json
{
  "routing_id": "route_20260119_135429",
  "timestamp": "2026-01-19T13:54:29+05:30",
  "user_request": "Create a new React web app with Docker deployment",
  "project_state": "IDLE",
  "detected_stack": "none",
  "primary_prompt": {
    "file": "prompts/by_entrypoint/new_project.txt",
    "reason": "New project detected",
    "compatibility": "compatible"
  },
  "supporting_prompts": [
    {
      "file": "prompts/examples_devops/docker_examples.md",
      "reason": "Docker mentioned in request",
      "compatibility": "compatible"
    }
  ],
  "rejected_prompts": [
    {
      "file": "prompts/by_entrypoint/existing_project.txt",
      "reason": "No existing codebase"
    }
  ],
  "estimated_token_cost": 8000
}
```

**Location**: `plans/Orchestration/routing_metadata.json`

## Version History

- **v1.0.0** (2026-01-19): Initial prompt routing algorithm
