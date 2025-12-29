#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>
#include <random>

namespace dnn::neural {

// Temporal Difference Learning (TD-Lambda)
class TemporalDifferenceLearning {
public:
    TemporalDifferenceLearning(int num_states, int num_actions, float gamma = 0.9f, 
                              float lambda = 0.8f, float alpha = 0.1f)
        : num_states_(num_states), num_actions_(num_actions),
          gamma_(gamma), lambda_(lambda), alpha_(alpha),
          gen_(std::random_device{}()) {
        
        // Initialize Q-table and eligibility traces
        q_table_.resize(num_states * num_actions, 0.0f);
        eligibility_.resize(num_states * num_actions, 0.0f);
    }
    
    // Select action using epsilon-greedy
    int select_action(int state, float epsilon = 0.1f) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        if (dist(gen_) < epsilon) {
            // Explore
            return gen_() % num_actions_;
        } else {
            // Exploit
            int best_action = 0;
            float best_value = get_q(state, 0);
            
            for (int a = 1; a < num_actions_; a++) {
                float value = get_q(state, a);
                if (value > best_value) {
                    best_value = value;
                    best_action = a;
                }
            }
            
            return best_action;
        }
    }
    
    // TD(λ) update
    void update(int state, int action, float reward, int next_state, int next_action) {
        // TD error: δ = r + γQ(s',a') - Q(s,a)
        float td_error = reward + gamma_ * get_q(next_state, next_action) - 
                        get_q(state, action);
        
        // Update eligibility trace for current state-action
        set_eligibility(state, action, get_eligibility(state, action) + 1.0f);
        
        // Update all Q-values and eligibility traces
        for (int s = 0; s < num_states_; s++) {
            for (int a = 0; a < num_actions_; a++) {
                // Q(s,a) += α * δ * e(s,a)
                float q = get_q(s, a);
                float e = get_eligibility(s, a);
                set_q(s, a, q + alpha_ * td_error * e);
                
                // Decay eligibility: e(s,a) *= γλ
                set_eligibility(s, a, e * gamma_ * lambda_);
            }
        }
    }
    
    // Reset eligibility traces
    void reset_eligibility() {
        eligibility_.assign(num_states_ * num_actions_, 0.0f);
    }
    
    // Get Q-value
    float get_q(int state, int action) const {
        int idx = state * num_actions_ + action;
        return (idx >= 0 && idx < static_cast<int>(q_table_.size())) ? q_table_[idx] : 0.0f;
    }
    
private:
    int num_states_;
    int num_actions_;
    float gamma_;   // Discount factor
    float lambda_;  // Trace decay
    float alpha_;   // Learning rate
    std::vector<float> q_table_;
    std::vector<float> eligibility_;
    std::mt19937 gen_;
    
    void set_q(int state, int action, float value) {
        int idx = state * num_actions_ + action;
        if (idx >= 0 && idx < static_cast<int>(q_table_.size())) {
            q_table_[idx] = value;
        }
    }
    
    float get_eligibility(int state, int action) const {
        int idx = state * num_actions_ + action;
        return (idx >= 0 && idx < static_cast<int>(eligibility_.size())) ? eligibility_[idx] : 0.0f;
    }
    
    void set_eligibility(int state, int action, float value) {
        int idx = state * num_actions_ + action;
        if (idx >= 0 && idx < static_cast<int>(eligibility_.size())) {
            eligibility_[idx] = value;
        }
    }
};

} // namespace dnn::neural
