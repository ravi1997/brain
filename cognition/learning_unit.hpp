#pragma once
#include <string>
#include <vector>

class LearningUnit {
public:
    void learn_interaction(const std::string&, const std::string&, double reward) {
        // In the future: Update RL Model or retrain LLM LoRA adapter.
        // For now: Log successful interaction.
        if (reward > 0) {
            // Save to knowledge graph or semantic memory
        }
    }
    
    void ingest_text(const std::string&) {
        // Parse text and update internal N-grams or embeddings
    }
};
