# Architecture: To-Be - Brain Replica

## Purpose
This document outlines the target architecture for the Brain Replica, incorporating planned security and cognitive enhancements (Milestones M2-M4).

## Target Architecture Model

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
        AUTH_MW[Auth Middleware]
    end

    subgraph "C++ Neural Processing (BRAIN)"
        SVR[Secure BrainServer - TLS Required]
        CORE[Brain Coordinator]
        
        REGIONS{Parallel Neural Regions}
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
        SEC[Security Module - Token Auth]
    end

    subgraph "Persistence Layer"
        PG[(PostgreSQL + pgvector)]
        RD[(Redis Semantic Cache)]
        SL[(Encrypted SQLite Storage)]
    end

    %% Connections
    HW <--> ROS
    ROS <--> SVR
    WEB <--> AUTH_MW
    AUTH_MW <--> BRIDGE
    CLI <--> SVR
    BRIDGE <--> SVR
    
    SVR <--> SEC
    SEC <--> CORE
    CORE <--> REGIONS
    CORE <--> EMO
    CORE <--> TASK
    CORE <--> SKIL
    
    MEMC <--> PG
    MEMC <--> RD
    MEMC <--> SL
```

## Key Architectural Changes (To-Be)

### 1. Security-by-Design (`M2`)
- **Token Injection**: A new `Security Module` in the C++ core will validate tokens for sensitive ports (9009, 9011).
- **TLS/SSL**: Implementation of encrypted communication between the Proxy and BrainServer.
- **RESTful Auth**: The Node.js Proxy will manage user sessions and tokens before bridging to the C++ layer.

### 2. Parallel Cognitive Processing (`M4`)
- **TBB/OpenMP Integration**: Neural regions (`Region`) will leverage multi-threading for the `process()` loop, allowing simultaneous encoding and memory retrieval.

### 3. Biological Memory Decay (`M3`)
- **Background Consolidator**: A background thread in `MemoryCenter` that runs periodic TTL cleanup on the Postgres store, simulating cognitive forgetting.

### 4. Semantic Cache Tier (`M4`)
- **Redis Integration**: Move from simple associative cache to a semantic vector cache in Redis, reducing Postgres load.
