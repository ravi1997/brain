#pragma once
#include <vector>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <cmath>

namespace dnn::distributed {

// Multi-Agent Reinforcement Learning
class MultiAgentRL {
public:
    struct Agent {
        std::string id;
        std::vector<float> q_table;  // Q-values
        float learning_rate;
        float discount_factor;
        float epsilon;  // Exploration rate
        
        Agent() : learning_rate(0.1f), discount_factor(0.9f), epsilon(0.1f) {}
        
        Agent(const std::string& agent_id, int state_space, int action_space,
              float lr = 0.1f, float gamma = 0.9f, float eps = 0.1f)
            : id(agent_id), learning_rate(lr), discount_factor(gamma), epsilon(eps) {
            q_table.resize(state_space * action_space, 0.0f);
        }
    };
    
    MultiAgentRL(int state_space_size, int action_space_size)
        : state_space_(state_space_size), action_space_(action_space_size),
          gen_(std::random_device{}()) {}
    
    // Add an agent to the system
    void add_agent(const std::string& agent_id, float lr = 0.1f, 
                   float gamma = 0.9f, float epsilon = 0.1f) {
        agents_[agent_id] = Agent(agent_id, state_space_, action_space_, lr, gamma, epsilon);
    }
    
    // Select action for an agent (epsilon-greedy)
    int select_action(const std::string& agent_id, int state) {
        auto it = agents_.find(agent_id);
        if (it == agents_.end()) {
            return 0;
        }
        
        Agent& agent = it->second;
        
        // Epsilon-greedy exploration
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        if (dist(gen_) < agent.epsilon) {
            // Explore: random action
            std::uniform_int_distribution<int> action_dist(0, action_space_ - 1);
            return action_dist(gen_);
        } else {
            // Exploit: best action
            return get_best_action(agent, state);
        }
    }
    
    // Update Q-value after taking action
    void update(const std::string& agent_id, int state, int action, 
               float reward, int next_state) {
        auto it = agents_.find(agent_id);
        if (it == agents_.end()) {
            return;
        }
        
        Agent& agent = it->second;
        
        // Q-learning update: Q(s,a) = Q(s,a) + α[r + γ*max(Q(s',a')) - Q(s,a)]
        int idx = state * action_space_ + action;
        int next_idx = next_state * action_space_ + get_best_action(agent, next_state);
        
        float current_q = agent.q_table[idx];
        float max_next_q = agent.q_table[next_idx];
        
        float td_error = reward + agent.discount_factor * max_next_q - current_q;
        agent.q_table[idx] += agent.learning_rate * td_error;
    }
    
    // Cooperative update: share rewards
    void cooperative_update(const std::vector<std::string>& agent_ids,
                           int state, const std::vector<int>& actions,
                           float shared_reward, int next_state) {
        for (size_t i = 0; i < agent_ids.size() && i < actions.size(); i++) {
            update(agent_ids[i], state, actions[i], shared_reward, next_state);
        }
    }
    
    // Competitive update: zero-sum rewards
    void competitive_update(const std::string& winner_id, const std::string& loser_id,
                           int state, int winner_action, int loser_action,
                           float reward_magnitude, int next_state) {
        update(winner_id, state, winner_action, reward_magnitude, next_state);
        update(loser_id, state, loser_action, -reward_magnitude, next_state);
    }
    
    // Get Q-value for state-action pair
    float get_q_value(const std::string& agent_id, int state, int action) const {
        auto it = agents_.find(agent_id);
        if (it == agents_.end()) {
            return 0.0f;
        }
        
        int idx = state * action_space_ + action;
        return it->second.q_table[idx];
    }
    
    // Decrease exploration rate (epsilon decay)
    void decay_epsilon(const std::string& agent_id, float decay_rate = 0.99f) {
        auto it = agents_.find(agent_id);
        if (it != agents_.end()) {
            it->second.epsilon *= decay_rate;
            it->second.epsilon = std::max(0.01f, it->second.epsilon);
        }
    }
    
    // Knowledge sharing: average Q-tables
    void share_knowledge(const std::vector<std::string>& agent_ids) {
        if (agent_ids.size() < 2) {
            return;
        }
        
        // Compute average Q-table
        std::vector<float> avg_q(state_space_ * action_space_, 0.0f);
        
        for (const auto& id : agent_ids) {
            auto it = agents_.find(id);
            if (it != agents_.end()) {
                for (size_t i = 0; i < avg_q.size(); i++) {
                    avg_q[i] += it->second.q_table[i];
                }
            }
        }
        
        for (auto& val : avg_q) {
            val /= agent_ids.size();
        }
        
        // Update each agent's Q-table (weighted average)
        float share_weight = 0.3f;  // 30% from shared knowledge
        for (const auto& id : agent_ids) {
            auto it = agents_.find(id);
            if (it != agents_.end()) {
                for (size_t i = 0; i < avg_q.size(); i++) {
                    it->second.q_table[i] = (1 - share_weight) * it->second.q_table[i] 
                                          + share_weight * avg_q[i];
                }
            }
        }
    }
    
    // Get agent count
    int get_agent_count() const {
        return agents_.size();
    }
    
    // Get best policy for an agent
    std::vector<int> get_policy(const std::string& agent_id) const {
        std::vector<int> policy;
        auto it = agents_.find(agent_id);
        
        if (it == agents_.end()) {
            return policy;
        }
        
        policy.reserve(state_space_);
        for (int s = 0; s < state_space_; s++) {
            policy.push_back(get_best_action(it->second, s));
        }
        
        return policy;
    }
    
private:
    int state_space_;
    int action_space_;
    std::unordered_map<std::string, Agent> agents_;
    std::mt19937 gen_;
    
    int get_best_action(const Agent& agent, int state) const {
        int best_action = 0;
        float best_value = agent.q_table[state * action_space_];
        
        for (int a = 1; a < action_space_; a++) {
            int idx = state * action_space_ + a;
            if (agent.q_table[idx] > best_value) {
                best_value = agent.q_table[idx];
                best_action = a;
            }
        }
        
        return best_action;
    }
};

} // namespace dnn::distributed
