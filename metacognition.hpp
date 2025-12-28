#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

namespace dnn {

class Metacognition {
public:
    struct Hyperparameters {
        double learning_rate = 0.01;
        double emotional_decay = 0.01;
        double curiosity_threshold = 0.5;
        double creativity = 0.3; // Temperature for generation
    };

    Hyperparameters params;

    void monitor_performance(double reward, double prediction_error) {
        recent_rewards.push_back(reward);
        if (recent_rewards.size() > 50) recent_rewards.erase(recent_rewards.begin());

        double avg_reward = 0.0;
        for (double r : recent_rewards) avg_reward += r;
        avg_reward /= recent_rewards.size();

        adjust_parameters(avg_reward, prediction_error);
    }

private:
    std::vector<double> recent_rewards;

    void adjust_parameters(double avg_reward, double error) {
        // If performance is poor (low reward), increase plasticity (learning rate) and creativity
        if (avg_reward < 0.2) {
            params.learning_rate = std::min(0.1, params.learning_rate * 1.05);
            params.creativity = std::min(0.9, params.creativity + 0.01);
        } else if (avg_reward > 0.8) {
            // Stabilize
            params.learning_rate = std::max(0.001, params.learning_rate * 0.95);
        }

        // If high prediction error, likely need more curiosity to explore
        if (error > 0.5) {
            params.curiosity_threshold = std::max(0.1, params.curiosity_threshold - 0.05); // Lower threshold = more curious
        }
    }
};

} // namespace dnn
