#pragma once
#include <vector>
#include <functional>
#include <algorithm>
#include <cmath>

namespace dnn::reasoning {

// Decision Theory with Expected Utility maximization
class DecisionTheory {
public:
    struct Outcome {
        std::string description;
        float utility;      // Value/payoff
        float probability;  // Likelihood
        
        Outcome() : utility(0), probability(0) {}
        Outcome(const std::string& desc, float u, float p)
            : description(desc), utility(u), probability(p) {}
    };
    
    struct Decision {
        std::string action;
        std::vector<Outcome> outcomes;
        float expected_utility;
        
        Decision() : expected_utility(0) {}
        Decision(const std::string& act) : action(act), expected_utility(0) {}
    };
    
    DecisionTheory() {}
    
    // Calculate expected utility for a decision
    float calculate_expected_utility(const Decision& decision) {
        float eu = 0.0f;
        
        for (const auto& outcome : decision.outcomes) {
            eu += outcome.probability * outcome.utility;
        }
        
        return eu;
    }
    
    // Choose best decision (maximize expected utility)
    Decision choose_best(std::vector<Decision>& decisions) {
        // Calculate expected utility for each
        for (auto& decision : decisions) {
            decision.expected_utility = calculate_expected_utility(decision);
        }
        
        // Find maximum
        auto best = std::max_element(decisions.begin(), decisions.end(),
            [](const Decision& a, const Decision& b) {
                return a.expected_utility < b.expected_utility;
            });
        
        return (best != decisions.end()) ? *best : Decision();
    }
    
    // Risk-averse decision (minimize variance)
    Decision choose_risk_averse(std::vector<Decision>& decisions) {
        for (auto& decision : decisions) {
            decision.expected_utility = calculate_expected_utility(decision);
            
            // Penalize variance
            float variance = 0.0f;
            for (const auto& outcome : decision.outcomes) {
                float diff = outcome.utility - decision.expected_utility;
                variance += outcome.probability * diff * diff;
            }
            
            decision.expected_utility -= 0.5f * std::sqrt(variance);  // Risk penalty
        }
        
        return choose_best(decisions);
    }
    
    // Minimax decision (minimize maximum loss)
    Decision choose_minimax(std::vector<Decision>& decisions) {
        Decision best;
        float best_worst_case = -std::numeric_limits<float>::infinity();
        
        for (auto& decision : decisions) {
            // Find worst outcome
            float worst = std::numeric_limits<float>::infinity();
            for (const auto& outcome : decision.outcomes) {
                worst = std::min(worst, outcome.utility);
            }
            
            if (worst > best_worst_case) {
                best_worst_case = worst;
                best = decision;
            }
        }
        
        return best;
    }
    
    // Maximum decision (maximize maximum gain)
    Decision choose_maximax(std::vector<Decision>& decisions) {
        Decision best;
        float best_best_case = -std::numeric_limits<float>::infinity();
        
        for (auto& decision : decisions) {
            // Find best outcome
            float maxval = -std::numeric_limits<float>::infinity();
            for (const auto& outcome : decision.outcomes) {
                maxval = std::max(maxval, outcome.utility);
            }
            
            if (maxval > best_best_case) {
                best_best_case = maxval;
                best = decision;
            }
        }
        
        return best;
    }
};

} // namespace dnn::reasoning
