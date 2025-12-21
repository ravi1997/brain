#pragma once
#include <string>
#include <map>
#include <algorithm>
#include <iostream>

struct EmotionalState {
    double happiness = 0.5; // 0-1
    double sadness = 0.0;   // 0-1
    double anger = 0.0;     // 0-1
    double fear = 0.0;      // 0-1
    double energy = 1.0;    // 0-1
    double boredom = 0.0;   // 0-1
    double curiosity = 0.5; // 0-1
};

class EmotionUnit {
public:
    EmotionalState state;

    EmotionUnit() = default;

    void update(double time_delta_seconds) {
        // Natural decay / homeostasis
        state.anger = std::max(0.0, state.anger - 0.01 * time_delta_seconds);
        state.fear = std::max(0.0, state.fear - 0.01 * time_delta_seconds);
        state.sadness = std::max(0.0, state.sadness - 0.005 * time_delta_seconds);
        
        // Energy consumption
        state.energy = std::max(0.0, state.energy - 0.001 * time_delta_seconds);
        
        // Boredom accumulation if idle
        // (Caller needs to reset boredom on activity)
        state.boredom = std::min(1.0, state.boredom + 0.005 * time_delta_seconds);
    }

    void stimulate(const std::string& emotion, double intensity) {
        if (emotion == "happiness") state.happiness = std::clamp(state.happiness + intensity, 0.0, 1.0);
        else if (emotion == "sadness") state.sadness = std::clamp(state.sadness + intensity, 0.0, 1.0);
        else if (emotion == "anger") state.anger = std::clamp(state.anger + intensity, 0.0, 1.0);
        else if (emotion == "fear") state.fear = std::clamp(state.fear + intensity, 0.0, 1.0);
        else if (emotion == "energy") state.energy = std::clamp(state.energy + intensity, 0.0, 1.0);
        else if (emotion == "boredom") state.boredom = std::clamp(state.boredom + intensity, 0.0, 1.0);
        else if (emotion == "curiosity") state.curiosity = std::clamp(state.curiosity + intensity, 0.0, 1.0);
    }

    std::string get_dominant_emotion() const {
        std::map<double, std::string> e = {
            {state.happiness, "Happy"},
            {state.sadness, "Sad"},
            {state.anger, "Angry"},
            {state.fear, "Afraid"},
            {state.boredom, "Bored"}
        };
        return std::max_element(e.begin(), e.end())->second;
    }
};
