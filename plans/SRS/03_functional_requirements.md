# SRS: Functional Requirements - Brain Replica

## 1. User Stories

| ID | Role | Feature | Reason | Acceptance Criteria |
|---|---|---|---|---|
| US-01 | User | Text Interaction | To communicate with the digital brain. | System responds on Port 9005 (Chat). |
| US-02 | Researcher | Emotional Monitoring | To understand the brain's internal state. | Port 9002 broadcasts JSON emotion data. |
| US-03 | Developer | Neural Skill Teaching | To expand the brain's capabilities without code changes. | `Learn:` command successfully creates a skill in `SkillManager`. |
| US-04 | User | Long-Term Memory | To have consistent conversations over many sessions. | Recalls facts stored in PostgreSQL via the interaction loop. |
| US-05 | Admin | System Control | To manage persistence and lifecycle. | Command `save` on Port 9009 triggers a DB dump. |

## 2. Core Features

### 2.1 Neural Interaction Loop
- **Req-F1.1**: The system shall vectorize input text using a N-GRAM boosted Bag-of-Words encoder.
- **Req-F1.2**: The system shall reinforce active pathways after every successful interaction (online learning).
- **Req-F1.3**: The system shall modulate response tone (caps, formality) based on the current `Happiness`, `Anger`, and `Sadness` levels.

### 2.2 Memory Management
- **Req-F2.1**: The system shall retain the last 10 interaction turns in Short-Term Memory (STM).
- **Req-F2.2**: The system shall perform semantic similarity search using `pgvector` for Long-Term Memory (LTM) retrieval.
- **Req-F2.3**: The system shall automatically save "interesting" interactions (heuristic-based) to the persistent store.

### 2.3 Task & Skill Execution
- **Req-F3.1**: The system shall support asynchronous task queuing with priorities.
- **Req-F3.2**: The system shall provide a dedicated skill query interface (`Do:` command).

## 3. Edge Cases
- **Low Energy**: If `energy < 0.2`, the system shall respond with "Sleepy" messages and defer complex processing.
- **Ambiguous Input**: If the decoder generates zero logits, a random "Philosophical" fallback message shall be used.
- **Memory Conflict**: In case of multiple similar LTM hits, the system shall prioritize the most recent entry.
