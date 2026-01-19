# Data Flow Diagram (DFD) - Brain Replica

## Purpose
Visualizes the flow of sensory data and thoughts from input to persistence and output.

## High-Level Data Flow

```mermaid
graph LR
    User([User/Hardware]) -- "Afferent Input (Text/Sensory)" --> Ports[TCP Ports 9005/9013]
    Ports -- "Raw Data" --> Brain[Brain Coordinator]
    
    subgraph "Neural Transformation"
        Brain -- "Tokens" --> NLU[NLU Encoder]
        NLU -- "Thought Vector" --> Cognition[Cognitive Core]
        Cognition -- "Context Query" --> Memory[Memory Center]
        Memory -- "Similarity Search" --> PG[(Postgres / pgvector)]
        PG -- "Fetched Facts" --> Memory
        Memory -- "Context Injection" --> Cognition
        Cognition -- "Response Vector" --> Decoder[NLU Decoder]
    end

    Decoder -- "Generated Text" --> Brain
    Brain -- "Efferent Signal" --> OutPort[TCP Ports 9005/9014]
    OutPort -- "Text/Motor Action" --> User
    
    subgraph "State & Management"
        Brain -- "Log Stream" --> LogPort[Port 9003]
        Brain -- "Mood Vector" --> EmoPort[Port 9002]
        Brain -- "State Save" --> File[(state/*.json)]
    end
```

## Detailed Transformation Steps

1. **Vectorization**: Input string is hashed and weighted into a $V \times 1$ sparse vector.
2. **Encoding**: NLU Regions transform sparse input into a dense $128 \times 1$ latent thought vector.
3. **Retrieval**: Thought vector triggers a Cosine Similarity query in Postgres via the pgvector extension.
4. **Synthesis**: Thought + Context + Emotions are fed into the Cognitive Center for decision making.
5. **Expression**: Result is decoded back to human-readable strings via a Softmax-like logits evaluation.
