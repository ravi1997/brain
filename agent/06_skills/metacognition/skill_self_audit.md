# Skill: Self-Audit & Meta-Improvement

## Purpose

Enables the Agent to evaluate its own performance and suggest optimizations to the Agent OS.

## Procedure

### 1. Failure Analysis

After any Tool Error (`E-XXXX`) or Gate Failure (`FAIL`):

1. **Root Cause**: Determine if the failure was due to:
   - Ambiguous instructions in a workflow.
   - Missing stack fingerprint.
   - Faulty logic in its own implementation.
2. **Persistence**: Has this happened before? (Check history or state).

### 2. Efficiency Review

During the "Release" phase of a milestone:

1. **Cycle Time**: Did any step take longer than expected?
2. **User Friction**: Did the user have to correct the Agent multiple times on the same topic?
3. **Draft Improvement**: Create a proposal for updating the relevant `agent/` file.

### 3. Updating the Agent Folder

1. **Propose**: Generate the new file content.
2. **Cross-Ref**: Check `00_system/06_templates_index.md` or `04_workflows/` to ensure no breakages.
3. **Execute**: Overwrite the file after getting `[USER_APPROVAL]`.

## STOP Condition

- Every identified inefficiency has a corresponding "Improvement Task" in the backlog.
- Agent OS remains internally consistent.
