# System Principle: Self-Evolution

## Purpose

Enables the Agent OS to improve its own instructions, gates, and workflows based on real-world performance and user feedback.

## Core Mandates

1. **Performance Journaling**: After every major milestone or release, the Agent must perform a "Post-Mortem" using `06_skills/metacognition/skill_self_audit.md`.
2. **Workflow Refinement**: If a workflow step (from `04_workflows/`) consistently leads to errors or user correction, the Agent MUST propose an edit to that workflow file.
3. **Template Versioning**: Templates in `07_templates/` should be updated if the output consistently lacks detail required by the next stage in the lifecycle.
4. **Knowledge Compound**: New stack signals or security patterns discovered during a project must be "upstreamed" into `02_detection/stack_fingerprints/` or `11_rules/stack_rules/`.

## Procedure for Self-Update

1. **Identify**: Notice a pattern of failure or inefficiency.
2. **Propose**: Generate a new version of the specific `agent/` file.
3. **Verify**: Use `profile_rule_checker.md` to ensure the update doesn't break cross-references.
4. **Human Approval**: The Agent MUST ask for approval before overwriting any file in the `agent/` folder itself.

## Constraint

- NEVER delete safety principles or security gates. Self-evolution is for efficiency and clarity, not for bypassing constraints.
