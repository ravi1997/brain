#pragma once
#include <unordered_map>
#include <string>
#include <cmath>

namespace dnn::distributed {

// Bayesian reputation system for agent trust
class ReputationSystem {
public:
    ReputationSystem(float alpha = 1.0f, float beta = 1.0f) 
        : default_alpha_(alpha), default_beta_(beta) {}
    
    void record_positive(const std::string& agent_id) {
        auto& rep = reputations_[agent_id];
        rep.first += 1.0f;  // alpha
    }
    
    void record_negative(const std::string& agent_id) {
        auto& rep = reputations_[agent_id];
        rep.second += 1.0f;  // beta
    }
    
    float get_reputation(const std::string& agent_id) const {
        auto it = reputations_.find(agent_id);
        if (it == reputations_.end()) {
            return default_alpha_ / (default_alpha_ + default_beta_);
        }
        float alpha = it->second.first + default_alpha_;
        float beta = it->second.second + default_beta_;
        return alpha / (alpha + beta);
    }
    
    bool is_trusted(const std::string& agent_id, float threshold = 0.7f) const {
        return get_reputation(agent_id) >= threshold;
    }
    
private:
    float default_alpha_;
    float default_beta_;
    std::unordered_map<std::string, std::pair<float, float>> reputations_;
};

} // namespace dnn::distributed
