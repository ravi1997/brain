#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <deque>
#include "dnn.hpp"

namespace dnn {

    // Child-like Actions
    enum class ActionType {
        IDLE = 0,
        SPEAK_BABBLE,  // Random phonemes/words
        SPEAK_INTENT,  // Try to convey specific need
        LISTEN,        // Focus on audio input
        EXPLORE,       // Look around / Read random memory
        USE_TOOL,      // Execute external tool
        SLEEP,         // Consolidate
        ACTION_COUNT   // Total number of actions
    };

    struct Experience {
        std::vector<double> state;
        int action;
        double reward;
        std::vector<double> next_state;
    };

    class CognitiveEngine {
    public:
        CognitiveEngine();
        ~CognitiveEngine();

        // Core RL Loop
        int decide_action(const std::vector<double>& state);
        void train(const std::vector<double>& state, int action, double reward, const std::vector<double>& next_state);
        
        // Manual override for teaching
        void force_learn(const std::vector<double>& state, int action, double reward);

        // Save/Load capabilities
        void save(const std::string& path);
        void load(const std::string& path);

    private:
        std::unique_ptr<dnn::NeuralNetwork> brain_policy_; // The DQN
        std::deque<Experience> replay_buffer_;
        std::mutex mutex_;
        
        // Hyperparameters
        double epsilon_ = 0.5; // Exploration rate (High for child)
        double gamma_ = 0.9;   // Discount factor
        double learning_rate_ = 0.01;
        
        static constexpr size_t INPUT_DIM = 64; // Compressed state vector size
        static constexpr size_t HIDDEN_DIM = 32;
        static constexpr size_t MAX_REPLAY_SIZE = 1000;
        
        int get_best_action(const std::vector<double>& state);
    };

} // namespace dnn
