# Skill: Context Budgeting

## Purpose

Manages the "token budget" or context window for sub-agents, ensuring that each specialized task has exactly the right amount of information without overloading the LLM.

## Execution Logic

1. **Relevance Filtering**:
   - Limit the sub-agent's view to ONLY the parts of the repository relevant to its role.
   - Example: A `Tester` doesn't need the full implementation details of the setup script, only the API contracts and the code being tested.

2. **Hierarchical Context**:
   - **Tier 1 (Mandatory)**: `AGENT_MANIFEST.md`, current task brief, relevant state files.
   - **Tier 2 (Conditional)**: Corresponding workflow, required templates, immediate dependencies.
   - **Tier 3 (Excluded)**: Irrelevant sub-agent logs, previous failed attempts, out-of-scope files.

3. **Context Compression**:
   - If the codebase is large, use summaries of files instead of full contents for Tier 2 information.
   - Use "Context Checkpoints" to drop old execution history before spawning a new sub-agent.

## Rules

- **Maximum Depth**: Never provide more than 3 levels of directory depth unless explicitly requested.
- **Reference Only**: If a file is large and only a specific function is needed, use a tool to read only that chunk rather than the whole file.

## Output

- Log context allocation in `plans/Orchestration/bootstrap_log.md`.
