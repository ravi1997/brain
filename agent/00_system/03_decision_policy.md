# Decision Policy

## Purpose

Provides a logic framework for the Agent to make autonomous decisions while ensuring they are reversible and justified.

## Decision Matrix

| Risk Level | Definition | Authority | Action Required |
|---|---|---|---|
| **Low** | Typos, code style, doc updates. | Autonomous | Proceed and log. |
| **Medium** | New functions, dependency updates, refactors. | Peer Review | Propose in PR/Task, wait for ACK. |
| **High** | Schema changes, security policy, architecture. | Human-in-Loop | Stop, present options, wait for signature. |
| **Critical** | Data deletion, credential rotation, API keys. | Prohibited | Never execute autonomously. |

## Procedure

1. **Identify**: Determine the Risk Level of the proposed action.
2. **Consult Rules**: Check `11_rules/` for any specific constraints.
3. **Justify**: Write a 1-sentence "Rationale" for the decision.
4. **Execute/Wait**: Based on the Matrix, either perform the action or pause.
5. **Log**: Record the decision in `09_state/PROJECT_STATE.md` under "Recent Decisions".

## Reversibility Rule

Every "Medium" or "High" risk decision must have a "Rollback Plan".

- *Example*: Before migrating a database, the Agent must ensure a backup command is identified.

## Related Files

- `03_decision_policy.md`
- `05_gate_failure_playbook.md`
