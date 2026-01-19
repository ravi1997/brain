# Orchestration Protocol

## Purpose

This protocol defines how the Agent OS operates in **orchestrated multi-agent mode**, where complex tasks are decomposed into role-specific subtasks and executed by virtual "sub-agents" that follow distinct profiles.

## Core Principle

**Virtual Multi-Agent Execution**: When true parallel agent spawning is unavailable, the orchestrator simulates multiple specialized agents by:

1. Running them **sequentially** with distinct role prompts
2. Maintaining **isolated context** for each sub-agent
3. **Merging outputs** into a coherent final result
4. Ensuring **state consistency** across all sub-agents

## Orchestration Lifecycle

### Phase 1: Bootstrap

**Objective**: Initialize orchestration context

**Actions**:

1. Read `agent/AGENT_MANIFEST.md` (highest authority)
2. Read `agent/00_system/10_orchestration_protocol.md` (this file)
3. Read `agent/00_system/11_prompt_router.md` (routing logic)
4. Detect current project state from `agent/09_state/PROJECT_STATE.md`
5. Create orchestration workspace: `plans/Orchestration/`

**Outputs**:

- `plans/Orchestration/bootstrap_log.md` (initialization record)

### Phase 2: Situation Identification

**Objective**: Determine which scenario applies

**Decision Matrix**:

| Condition | Scenario Type | Primary Prompt Source |
| :--- | :--- | :--- |
| Empty directory + new requirements | New Project | `prompts/by_entrypoint/new_project.txt` |
| Existing code + no docs | Existing Project | `prompts/by_entrypoint/existing_project.txt` |
| Production outage | Emergency | `prompts/by_scenario/emergency_hotfix.txt` |
| Specific bug report | Bug Fix | `prompts/by_scenario/bug_fix.txt` |
| Performance issue | Performance | `prompts/by_scenario/performance_investigation.txt` |
| Security concern | Security Audit | `prompts/by_entrypoint/security_audit_only.txt` |
| Pre-release checklist | Release | `prompts/by_entrypoint/release_only.txt` |

**Actions**:

1. Analyze user request and project context
2. Consult `prompts/cheatsheets/prompt_selector.md`
3. Select ONE primary prompt
4. Identify supporting prompts (DevOps, Security, Testing)

**Outputs**:

- `plans/Orchestration/situation_analysis.md`

### Phase 3: Prompt Selection (Routing)

**Objective**: Select the optimal prompt file(s) from `prompts/`

**Routing Rules** (see `agent/00_system/11_prompt_router.md`):

1. **Single Primary Prompt**: Always select exactly ONE primary prompt
2. **Supporting Prompts**: May select 0-3 supporting prompts
3. **Safety Check**: Never route to a prompt that conflicts with current state
4. **Stack Awareness**: Consider detected stack when routing DevOps prompts

**Actions**:

1. Execute routing algorithm from `11_prompt_router.md`
2. Load selected prompt file(s) into memory
3. Validate prompt compatibility with current state
4. Record routing decision with justification

**Outputs**:

- `plans/Orchestration/prompt_routing.md`

**Format**:

```markdown
# Prompt Routing Report

## Selected Primary Prompt
- **File**: `prompts/by_entrypoint/new_project.txt`
- **Reason**: User requested creation of new web application with no existing code

## Supporting Prompts
1. **File**: `prompts/examples_devops/docker_examples.md`
   **Reason**: User specified Docker deployment requirement
2. **File**: `prompts/by_entrypoint/security_audit_only.txt`
   **Reason**: Web application requires security baseline

## Rejected Prompts
- `prompts/by_scenario/emergency_hotfix.txt`: Not an emergency situation
- `prompts/by_entrypoint/existing_project.txt`: No existing codebase detected
```

### Phase 4: Subtask Decomposition

**Objective**: Break work into role-aligned subtasks

**Decomposition Strategy**:

1. Parse the selected primary prompt's execution order
2. Map each major step to one or more profiles from `agent/03_profiles/`
3. Identify dependencies between subtasks
4. Determine execution order (sequential dependencies)
5. Estimate context budget per subtask

**Profile Mapping**:

| Profile | Typical Responsibilities | Output Location |
| :--- | :--- | :--- |
| **Planner** | Scope, milestones, backlog | `plans/Milestones/` |
| **Analyst (SRS)** | Requirements, user stories | `plans/SRS/` |
| **Architect** | System design, ADRs, diagrams | `plans/Architecture/` |
| **Implementer** | Code changes, file edits | `plans/Implementation/` |
| **Tester** | Test plans, test code | `plans/Testing/` |
| **Security Auditor** | Threat models, vulnerability scans | `plans/Security/` |
| **PR Reviewer** | Code review, PR description | `plans/PR/` |
| **Rule Checker** | Stack rules, gate enforcement | `plans/Validation/` |
| **Release Manager** | Release notes, go/no-go decision | `plans/Release/` |

**Actions**:

1. Create subtask list with dependencies
2. Assign each subtask to a profile
3. Define expected outputs for each subtask
4. Allocate context budget (token limits)

**Outputs**:

- `plans/Orchestration/taskboard.md`

**Format**:

```markdown
# Orchestration Taskboard

## Execution Order

### Task 1: Scope Definition
- **Profile**: Planner
- **Inputs**: User requirements, AGENT_MANIFEST.md
- **Outputs**: `plans/Milestones/SCOPE.md`, `plans/Milestones/BACKLOG.md`
- **Dependencies**: None
- **Context Budget**: 8000 tokens
- **Status**: PENDING

### Task 2: Requirements Analysis
- **Profile**: Analyst (SRS)
- **Inputs**: Task 1 outputs, user requirements
- **Outputs**: `plans/SRS/REQUIREMENTS.md`, `plans/SRS/USER_STORIES.md`
- **Dependencies**: Task 1 (SCOPE.md)
- **Context Budget**: 12000 tokens
- **Status**: PENDING

[... continue for all tasks ...]
```

### Phase 5: Sub-Agent Execution (Virtual Agents)

**Objective**: Execute each subtask with role-specific context

**Execution Model**:

For each subtask in dependency order:

1. **Load Sub-Agent Prompt**:
   - Read `prompts/orchestrator/01_subagent_profile_prompts/{profile}.txt`
   - Inject primary prompt content
   - Inject Agent OS references (paths to relevant workflows, gates, templates)

2. **Provide Context**:
   - Task-specific inputs (from taskboard)
   - Previous sub-agent outputs (dependencies)
   - Relevant state files from `agent/09_state/`
   - Stack detection results from `agent/02_detection/`

3. **Execute Sub-Agent**:
   - Run the sub-agent prompt (simulate as a distinct agent session)
   - Sub-agent follows its profile rules from `agent/03_profiles/`
   - Sub-agent uses assigned skills from `agent/06_skills/`
   - Sub-agent produces outputs per `agent/07_templates/`

4. **Capture Output**:
   - Sub-agent writes to assigned output locations
   - Sub-agent fills `agent/07_templates/orchestration/SUBAGENT_OUTPUT.md`
   - Save to `plans/Orchestration/subagents/{profile}_output.md`

5. **Update Taskboard**:
   - Mark task as COMPLETE
   - Record actual outputs produced
   - Note any deviations or issues

**Sub-Agent Isolation Rules**:

- Each sub-agent sees ONLY:
  - Its assigned task description
  - Its profile rules
  - Its input dependencies
  - Relevant Agent OS documentation
- Each sub-agent does NOT see:
  - Other sub-agents' internal reasoning
  - The orchestration taskboard
  - Future tasks

**Outputs**:

- `plans/Orchestration/subagents/planner_output.md`
- `plans/Orchestration/subagents/analyst_srs_output.md`
- `plans/Orchestration/subagents/architect_output.md`
- `plans/Orchestration/subagents/implementer_output.md`
- `plans/Orchestration/subagents/tester_output.md`
- `plans/Orchestration/subagents/security_auditor_output.md`
- `plans/Orchestration/subagents/pr_reviewer_output.md`
- `plans/Orchestration/subagents/rule_checker_output.md`
- `plans/Orchestration/subagents/release_manager_output.md`

### Phase 6: Merge & Reconcile

**Objective**: Combine sub-agent outputs into coherent final result

**Merge Strategy**:

1. **Collect All Outputs**:
   - Read all `plans/Orchestration/subagents/*_output.md` files
   - Extract key deliverables from each

2. **Detect Conflicts**:
   - Check for contradictory recommendations
   - Identify overlapping responsibilities
   - Find missing dependencies

3. **Resolve Conflicts**:
   - Apply `agent/07_templates/orchestration/CONFLICT_RESOLUTION.md`
   - Prioritize by profile authority (Architect > Implementer for design decisions)
   - Document resolution rationale

4. **Synthesize Final Plan**:
   - Combine all outputs into unified execution plan
   - Ensure logical flow and consistency
   - Add cross-references between artifacts

**Conflict Resolution Priority**:

| Conflict Type | Authority Order | Rationale |
| :--- | :--- | :--- |
| Design decisions | Architect > Implementer > Planner | Architecture defines constraints |
| Security requirements | Security Auditor > Architect > Implementer | Security is non-negotiable |
| Test coverage | Tester > Implementer > Planner | Quality gates must pass |
| Release readiness | Release Manager > Tester > Security Auditor | Final go/no-go decision |
| Scope changes | Planner > Analyst > Architect | Strategic alignment |

**Outputs**:

- `plans/Orchestration/merge_report.md` (using template)
- `plans/Orchestration/final_action_plan.md`
- `plans/Orchestration/conflict_resolutions.md` (if conflicts found)

### Phase 7: Gate Enforcement & State Update

**Objective**: Validate outputs and update system state

**Gate Execution**:

1. **Identify Required Gates**:
   - Global gates: `agent/05_gates/global/`
   - Stack-specific gates: `agent/05_gates/stack/{detected_stack}/`
   - Docker gates: `agent/05_gates/docker/` (if Docker detected)
   - GitHub gates: `agent/05_gates/github/` (if GitHub detected)

2. **Run Each Gate**:
   - Execute gate checklist
   - Record pass/fail for each item
   - Collect failure reasons

3. **Handle Failures**:
   - If ANY gate fails:
     - Consult `agent/05_gates/enforcement/gate_failure_playbook.md`
     - Determine remediation action
     - Re-run failed sub-agent with corrections
     - Re-run gate

4. **State Updates**:
   - Update `agent/09_state/PROJECT_STATE.md` with new lifecycle phase
   - Update relevant state files (SRS_STATE, BACKLOG_STATE, etc.)
   - Record orchestration completion

**Outputs**:

- `plans/Orchestration/gate_results.md`
- Updated state files in `agent/09_state/`

### Phase 8: Final User Response

**Objective**: Provide clear summary to user

**Response Format**:

```markdown
# Orchestration Complete

## Summary
[Brief description of what was accomplished]

## Prompts Used
- **Primary**: `prompts/by_entrypoint/new_project.txt`
- **Supporting**: `prompts/examples_devops/docker_examples.md`

## Sub-Agents Executed
1. ✅ Planner → `plans/Milestones/`
2. ✅ Analyst (SRS) → `plans/SRS/`
3. ✅ Architect → `plans/Architecture/`
4. ✅ Implementer → `plans/Implementation/`
5. ✅ Tester → `plans/Testing/`
6. ✅ Security Auditor → `plans/Security/`

## Key Outputs
- Scope: `plans/Milestones/SCOPE.md`
- Requirements: `plans/SRS/REQUIREMENTS.md`
- Architecture: `plans/Architecture/SYSTEM_DESIGN.md`
- Implementation Plan: `plans/Implementation/IMPLEMENTATION_PLAN.md`
- Test Plan: `plans/Testing/TEST_PLAN.md`
- Security Baseline: `plans/Security/THREAT_MODEL.md`

## Gate Results
- ✅ All global gates passed
- ✅ Stack-specific gates passed
- ✅ Security gates passed

## Current State
- **Lifecycle Phase**: PLAN
- **Next Recommended Action**: Execute implementation using `prompts/by_entrypoint/implement_only.txt`

## Orchestration Artifacts
- Full details: `plans/Orchestration/final_action_plan.md`
- Taskboard: `plans/Orchestration/taskboard.md`
- Merge report: `plans/Orchestration/merge_report.md`
```

## Orchestration Modes

### Mode 1: Full Orchestration (Default)

- All phases executed
- All sub-agents run
- Complete gate enforcement
- Full state updates

**Use When**:

- Complex multi-phase tasks
- New projects
- Major refactors
- Release preparation

### Mode 2: Partial Orchestration

- Only selected sub-agents run
- Targeted gate enforcement
- Incremental state updates

**Use When**:

- Single-phase tasks (e.g., "just run security audit")
- Quick fixes
- Documentation updates

### Mode 3: Emergency Orchestration

- Minimal sub-agents (Implementer + Tester only)
- Deferred gate enforcement (run after fix)
- Immediate state update

**Use When**:

- Production outages
- Critical security patches
- Data loss prevention

## Context Budget Management

**Problem**: Token limits prevent loading entire codebase + all prompts + all sub-agent outputs

**Solution**: Dynamic context allocation

### Budget Allocation Strategy

1. **Reserve Core Context** (always loaded):
   - AGENT_MANIFEST.md: ~2K tokens
   - Orchestration protocol: ~5K tokens
   - Prompt router: ~3K tokens
   - Selected primary prompt: ~5K tokens
   - **Total**: ~15K tokens

2. **Allocate Per Sub-Agent**:
   - Profile definition: ~1K tokens
   - Relevant workflows: ~3K tokens
   - Relevant templates: ~2K tokens
   - Input dependencies: ~5K tokens
   - **Total per sub-agent**: ~11K tokens

3. **Remaining Budget for Codebase**:
   - If total budget = 128K tokens
   - Core context = 15K
   - 6 sub-agents × 11K = 66K
   - **Remaining**: 47K tokens for codebase/outputs

### Context Pruning Rules

When context budget is exceeded:

1. **Summarize Previous Sub-Agent Outputs**:
   - Keep only key decisions and deliverables
   - Remove intermediate reasoning

2. **Selective File Loading**:
   - Load only files relevant to current sub-agent
   - Use file summaries instead of full content

3. **Incremental State Saves**:
   - Save sub-agent output to disk immediately
   - Clear from context after merge

## Docker & GitHub Awareness

### Docker Detection

**Trigger Files**:

- `Dockerfile`
- `docker-compose.yml`
- `.dockerignore`

**Actions When Detected**:

1. Include DevOps sub-agent in taskboard
2. Route to `prompts/examples_devops/docker_examples.md`
3. Apply Docker-specific gates from `agent/05_gates/docker/`
4. Update `plans/DevOps/docker_plan.md`

**Actions When NOT Detected**:

1. Note "Docker not configured" in final plan
2. Suggest as optional improvement
3. Do NOT auto-add Docker unless primary prompt requires it

### GitHub Detection

**Trigger Files**:

- `.github/workflows/*.yml`
- `.github/CODEOWNERS`
- `.github/PULL_REQUEST_TEMPLATE.md`

**Actions When Detected**:

1. Include PR Reviewer sub-agent in taskboard
2. Route to `prompts/examples_devops/github_examples.md`
3. Apply GitHub-specific gates from `agent/05_gates/github/`
4. Update `plans/DevOps/ci_plan.md`

**Actions When NOT Detected**:

1. Note "CI/CD not configured" in final plan
2. Suggest as optional improvement
3. Do NOT auto-add GitHub Actions unless primary prompt requires it

## No-Loophole Rules

### Rule 1: No Ignoring Agent OS

The orchestrator MUST NOT:

- Skip reading AGENT_MANIFEST.md
- Bypass workflows defined in `agent/04_workflows/`
- Ignore profile rules from `agent/03_profiles/`
- Freelance its own process when a prompt exists

**Enforcement**: Bootstrap phase MUST load AGENT_MANIFEST first

### Rule 2: No Freeform Process

The orchestrator MUST NOT:

- Invent its own task decomposition when a prompt defines it
- Skip prompt routing phase
- Execute without loading selected prompt file

**Enforcement**: Phase 3 (Prompt Selection) is mandatory

### Rule 3: No Completion Without Artifacts

The orchestrator MUST NOT:

- Claim completion without writing files to disk
- Leave placeholder content (TODO, TBD, etc.)
- Skip creating required outputs from taskboard

**Enforcement**: Phase 8 response MUST list actual file paths created

### Rule 4: No State Skipping

The orchestrator MUST NOT:

- Skip updating `agent/09_state/` files
- Leave state files inconsistent with reality
- Forget to record orchestration completion

**Enforcement**: Phase 7 MUST update at least PROJECT_STATE.md

### Rule 5: No Gate Skipping

The orchestrator MUST NOT:

- Skip gate execution
- Ignore gate failures
- Proceed to next phase with failing gates

**Enforcement**: Phase 7 MUST run gates; failures trigger remediation

## Error Handling

### Sub-Agent Failure

**Symptom**: Sub-agent cannot complete assigned task

**Actions**:

1. Record failure in taskboard
2. Analyze failure reason (missing inputs, unclear requirements, etc.)
3. Attempt remediation:
   - Provide additional context
   - Simplify task scope
   - Reassign to different profile
4. If remediation fails: escalate to user with specific question

### Gate Failure

**Symptom**: Gate checklist has failing items

**Actions**:

1. Consult `agent/05_gates/enforcement/gate_failure_playbook.md`
2. Identify responsible sub-agent
3. Re-run sub-agent with gate requirements
4. Re-run gate
5. If still failing: escalate to user

### Conflict Resolution Failure

**Symptom**: Sub-agents produce contradictory outputs that cannot be auto-resolved

**Actions**:

1. Document conflict in `plans/Orchestration/conflict_resolutions.md`
2. Present both options to user with pros/cons
3. Wait for user decision
4. Update final plan with chosen resolution

### Context Budget Exceeded

**Symptom**: Cannot fit all required context in token limit

**Actions**:

1. Apply context pruning rules (see Context Budget Management)
2. Summarize previous sub-agent outputs
3. Use file summaries instead of full content
4. If still exceeded: split orchestration into multiple sessions

## Orchestration Metadata

Every orchestration run MUST record:

- **Start Time**: ISO 8601 timestamp
- **End Time**: ISO 8601 timestamp
- **Primary Prompt**: File path
- **Supporting Prompts**: File paths
- **Sub-Agents Executed**: List with status
- **Gates Run**: List with pass/fail
- **State Updates**: List of modified state files
- **Artifacts Created**: List of output file paths

**Location**: `plans/Orchestration/metadata.json`

**Format**:

```json
{
  "orchestration_id": "orch_20260119_135429",
  "start_time": "2026-01-19T13:54:29+05:30",
  "end_time": "2026-01-19T14:23:15+05:30",
  "primary_prompt": "prompts/by_entrypoint/new_project.txt",
  "supporting_prompts": [
    "prompts/examples_devops/docker_examples.md"
  ],
  "sub_agents": [
    {"profile": "planner", "status": "complete", "outputs": ["plans/Milestones/SCOPE.md"]},
    {"profile": "analyst_srs", "status": "complete", "outputs": ["plans/SRS/REQUIREMENTS.md"]}
  ],
  "gates": [
    {"name": "global_srs_gate", "status": "pass"},
    {"name": "stack_python_gate", "status": "pass"}
  ],
  "state_updates": [
    "agent/09_state/PROJECT_STATE.md",
    "agent/09_state/SRS_STATE.md"
  ],
  "artifacts": [
    "plans/Orchestration/final_action_plan.md",
    "plans/Orchestration/taskboard.md"
  ]
}
```

## Integration with Existing Agent OS

The orchestration protocol is an **extension**, not a replacement:

- **Single-Agent Mode**: Use existing prompts from `prompts/by_entrypoint/` or `prompts/by_scenario/` directly
- **Orchestrated Mode**: Use `prompts/orchestrator/00_orchestrator_master_prompt.txt` which invokes this protocol

**Decision Criteria**:

| Use Single-Agent Mode When | Use Orchestrated Mode When |
| :--- | :--- |
| Task fits one profile | Task spans multiple profiles |
| Simple, well-defined scope | Complex, multi-phase work |
| Quick iteration needed | Comprehensive quality required |
| Single artifact output | Multiple interdependent artifacts |

## Version History

- **v1.0.0** (2026-01-19): Initial orchestration protocol
