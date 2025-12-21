#pragma once
#include <string>
#include <vector>
#include "emotion_unit.hpp"

enum class ActionType {
    REPLY,
    RESEARCH,
    IGNORE,
    SLEEP,
    INITIATE_TASK
};

struct Decision {
    ActionType type;
    std::string payload;
    double confidence;
};

class DecisionUnit {
public:
    Decision decide(const std::string& input_text, const EmotionalState& emotion, double energy) {
        Decision d;
        d.confidence = 1.0;

        // Basic Decision Logic (Will be enhanced)
        if (energy < 0.1) {
            d.type = ActionType::SLEEP;
            d.payload = "Low energy. Initiating sleep mode.";
            return d;
        }

        if (input_text.empty()) {
            if (emotion.boredom > 0.8) {
                d.type = ActionType::INITIATE_TASK;
                d.payload = "I am bored. I will find something to do.";
            } else {
                d.type = ActionType::IGNORE;
            }
            return d;
        }

        if (input_text.find("research") != std::string::npos || input_text.find("learn") != std::string::npos) {
            d.type = ActionType::RESEARCH;
            d.payload = input_text; // Extract topic ideally
        } else {
            d.type = ActionType::REPLY;
            d.payload = input_text;
        }

        return d;
    }
};
