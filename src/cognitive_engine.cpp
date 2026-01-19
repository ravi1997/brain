#include "cognitive_engine.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <random>
#include <algorithm>
#include <cmath>

namespace dnn {

    CognitiveEngine::CognitiveEngine() {
        // Initialize simple DQN: Input -> Hidden -> Actions
        std::vector<size_t> topology = {INPUT_DIM, HIDDEN_DIM, (size_t)ActionType::ACTION_COUNT};
        brain_policy_ = std::make_unique<dnn::NeuralNetwork>(topology);
        
        std::cout << "[CognitiveEngine]: Initialized RL Brain (Child Mode). Epsilon=" << epsilon_ << std::endl;
    }

    CognitiveEngine::~CognitiveEngine() {}

    int CognitiveEngine::decide_action(const std::vector<double>& state) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Epsilon-Greedy Exploration
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

        if (dis(gen) < epsilon_) {
            // Explore: Random Action
            std::uniform_int_distribution<> action_dis(0, (int)ActionType::ACTION_COUNT - 1);
            return action_dis(gen);
        }

        return get_best_action(state);
    }

    int CognitiveEngine::get_best_action(const std::vector<double>& state) {
        std::vector<double> q_values = brain_policy_->predict(state);
        
        // Find argmax
        int best_action = 0;
        double max_val = -1e9;
        for (size_t i = 0; i < q_values.size(); ++i) {
            if (q_values[i] > max_val) {
                max_val = q_values[i];
                best_action = (int)i;
            }
        }
        return best_action;
    }

    void CognitiveEngine::train(const std::vector<double>& state, int action, double reward, const std::vector<double>& next_state) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 1. Add to Experience Replay
        if (replay_buffer_.size() >= MAX_REPLAY_SIZE) {
            replay_buffer_.pop_front();
        }
        replay_buffer_.push_back({state, action, reward, next_state});

        // 2. Sample random batch (Mini-batch size 16)
        // Simplified: Just train on the *current* experience + 5 random past memories
        std::vector<Experience> batch;
        batch.push_back({state, action, reward, next_state});
        
        if (replay_buffer_.size() > 10) {
             for(int i=0; i<5; ++i) {
                 batch.push_back(replay_buffer_[rand() % replay_buffer_.size()]);
             }
        }

        // 3. Q-Learning Update
        for (const auto& exp : batch) {
            // Target Q = Reward + Gamma * Max(Q(next_state))
            std::vector<double> next_q_values = brain_policy_->predict(exp.next_state);
            double max_next_q = -1e9;
            for(double v : next_q_values) if(v > max_next_q) max_next_q = v;
            
            // Current Q values
            std::vector<double> current_q = brain_policy_->predict(exp.state);
            
            // Update the Q-value for the action taken
            double target = exp.reward + gamma_ * max_next_q;
            
            // In a real framework, we'd pass 'target' vector. Here we have to construct it.
            // We want the network to output 'target' for 'action' index, effectively.
            // Error = (target - current_q[action])
            // We'll trust the dnn::NeuralNetwork::train to handle backprop if we give it the target vector.
            current_q[exp.action] = target; 
            
            brain_policy_->train({exp.state}, {current_q}, 1, 1, learning_rate_);
        }
        
        // Decay exploration
        if (epsilon_ > 0.05) epsilon_ *= 0.9995;
    }

    void CognitiveEngine::force_learn(const std::vector<double>& state, int action, double reward) {
        // Direct reinforcement (Teacher Mode)
        // We simulate a transition to self (next_state = state) for immediate gratification mapping
        train(state, action, reward, state);
    }

    void CognitiveEngine::save(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ofstream os(path, std::ios::binary);
        if (os) {
            brain_policy_->save(os);
        }
    }

    void CognitiveEngine::load(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ifstream is(path, std::ios::binary);
        if (is) {
            brain_policy_->load(is);
        }
    }

} // namespace dnn
