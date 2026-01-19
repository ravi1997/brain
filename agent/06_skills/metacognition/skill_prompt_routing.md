# Skill: Prompt Routing

## Purpose

Enables the Agent to programmatically select the most appropriate execution context (prompt) based on the user's specific request and the current repository state.

## Execution Logic

1. **Context Extraction**:
   - Analyze user intent (e.g., "fix a bug", "new project").
   - Identifying technical constraints (e.g., "Next.js", "Docker").
   - Check `agent/09_state/` for the current lifecycle phase.

2. **Categorization**:
   - Match intent against categories in `prompts/cheatsheets/prompt_selector.md`.
   - Determine if the situation is an "Emergency" (overrides standard routing).

3. **Selection**:
   - **Primary**: Select one core workflow prompt (e.g., `prompts/by_entrypoint/new_project.txt`).
   - **Secondary**: Select auxiliary prompts (e.g., `prompts/examples_devops/docker_examples.md`) if specific tech is detected.

4. **Validation**:
   - Verify that the selected prompt's required artifacts don't conflict with current state.
   - Example: Don't route to `new_project.txt` if an SRS already exists and the state is `PLAN`.

## Criteria for Selection

- **Specificity**: Scenario-specific prompts (`by_scenario/`) take precedence over entrypoints (`by_entrypoint/`) if a match is > 80% certain.
- **Safety**: High-risk tasks (data migration, deletion) must include the `security_audit_only.txt` as a required secondary prompt.

## Output

- Update `plans/Orchestration/prompt_routing.md`.
