# Architectural Overview - Brain Replica

## System Decomposition

```mermaid
graph TD
    subgraph "External/Hardware Layer"
        HW[SENSORS/ACTUATORS]
        ROS[ROS 2 Bridge]
    end

    subgraph "Network & Proxy Layer"
        CLI[TCP Clients]
        WEB[React Dashboard]
        BRIDGE[Node.js Proxy]
    end

    subgraph "C++ Neural Processing (BRAIN)"
        SVR[BrainServer - Port 9001-9014]
        CORE[Brain Coordinator]
        
        REGIONS{Neural Regions}
        ENCO[Language Encoder]
        DECO[Language Decoder]
        COGN[Cognitive Center]
        MEMC[Memory Center]
        
        REGIONS --- ENCO
        REGIONS --- DECO
        REGIONS --- COGN
        REGIONS --- MEMC
        
        EMO[Emotion Engine]
        TASK[Task Manager]
        SKIL[Skill Manager]
    end

    subgraph "Persistence Layer"
        PG[(PostgreSQL + pgvector)]
        RD[(Redis Cache)]
        SL[(SQLite Storage)]
    end

    %% Connections
    HW <--> ROS
    ROS <--> SVR
    WEB <--> BRIDGE
    CLI <--> SVR
    BRIDGE <--> SVR
    
    SVR <--> CORE
    CORE <--> REGIONS
    CORE <--> EMO
    CORE <--> TASK
    CORE <--> SKIL
    
    MEMC <--> PG
    MEMC <--> RD
    MEMC <--> SL
```

## Data Model (ERD)

```mermaid
erDiagram
    BRAIN_KV_STORE {
        string key PK
        string value
        vector_384 embedding "HNSW Indexed"
    }
    
    REFLEX_STORE {
        string trigger PK
        string response
        float reward_weight
    }
    
    TASK_QUEUE {
        uuid id PK
        string description
        string type
        int priority
        timestamp created_at
    }
    
    BRAIN_KV_STORE ||--o{ TASK_QUEUE : "referenced in"
```

## Sequence: Interaction Loop

```mermaid
sequenceDiagram
    participant User
    participant SVR as BrainServer
    participant CORE as Brain
    participant NLU as LanguageEncoder
    participant MEM as MemoryCenter
    participant DECO as LanguageDecoder
    participant OUT as OutputPort (9014)

    User->>SVR: Afferent Input (9013)
    SVR->>CORE: process_interaction(input)
    CORE->>NLU: vectorize(text)
    NLU-->>CORE: thought_vector
    CORE->>MEM: query_assoc(thought_vector)
    MEM-->>CORE: memory_context
    CORE->>DECO: generate(thought + context)
    DECO-->>CORE: response_text
    CORE->>SVR: broadcast_output(response_text)
    SVR->>OUT: Speak (9014)
```

## Interface Specifications

### 1. Neural Interface (`Region`)
- `process(input_vector)`: Normalizes and propagates through the network.
- `reinforce()`: Updates synaptic weights based on recently active neurons.

### 2. Storage Interface (`MemoryStore`)
- `store(key, value, embedding)`: Persistent record.
- `query(search_term)`: RAG lookup.
- `forget(ttl)`: Maintenance logic.

### 3. Server Interface (`TcpServer`)
- `start()`: Bind and listen.
- `broadcast(msg)`: Relay to all listeners.
- `on_input(callback)`: Hook for afferent data.
