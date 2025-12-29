#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace dnn::reasoning {

// Non-Monotonic Reasoning - reasoning that can be retracted
class NonMonotonicReasoning {
public:
    struct Belief {
        std::string proposition;
        float strength;  // 0-1
        std::vector<std::string> supports;  // What supports this belief
        
        Belief() : strength(0.5f) {}
        Belief(const std::string& p, float s = 0.5f) : proposition(p), strength(s) {}
    };
    
    struct Defeater {
        std::string defeater_proposition;
        std::string defeated_proposition;
        float strength;
        
        Defeater() : strength(1.0f) {}
        Defeater(const std::string& defeater, const std::string& defeated, float s = 1.0f)
            : defeater_proposition(defeater), defeated_proposition(defeated), strength(s) {}
    };
    
    NonMonotonicReasoning() {}
    
    // Add belief to knowledge base
    void add_belief(const Belief& belief) {
        beliefs_[belief.proposition] = belief;
    }
    
    // Add defeater: if defeater holds, defeated is weakened/retracted
    void add_defeater(const Defeater& defeater) {
        defeaters_.push_back(defeater);
    }
    
    // Query belief strength (considering defeaters)
    float query(const std::string& proposition) {
        if (!beliefs_.count(proposition)) {
            return 0.0f;
        }
        
        float base_strength = beliefs_[proposition].strength;
        float defeated_amount = 0.0f;
        
        // Check all defeaters
        for (const auto& defeater : defeaters_) {
            if (defeater.defeated_proposition == proposition) {
                // Does the defeater hold?
                if (beliefs_.count(defeater.defeater_proposition)) {
                    float defeater_strength = beliefs_[defeater.defeater_proposition].strength;
                    defeated_amount += defeater_strength * defeater.strength;
                }
            }
        }
        
        // Reduce belief strength by defeaters
        float final_strength = base_strength * (1.0f - std::min(1.0f, defeated_amount));
        return std::max(0.0f, final_strength);
    }
    
    // Update belief (non-monotonic: can decrease)
    void update_belief(const std::string& proposition, float new_strength) {
        if (beliefs_.count(proposition)) {
            beliefs_[proposition].strength = new_strength;
        } else {
            add_belief(Belief(proposition, new_strength));
        }
    }
    
    // Retract belief (set strength to 0)
    void retract(const std::string& proposition) {
        update_belief(proposition, 0.0f);
    }
    
    // Argumentation-based reasoning
    struct Argument {
        std::vector<std::string> premises;
        std::string conclusion;
        float strength;
        
        Argument() : strength(1.0f) {}
    };
    
    void add_argument(const Argument& arg) {
        arguments_.push_back(arg);
    }
    
    // Compute justified beliefs using argumentation
    std::unordered_map<std::string, float> compute_justified_beliefs() {
        std::unordered_map<std::string, float> justified;
        
        // For each argument
        for (const auto& arg : arguments_) {
            // Check if premises are justified
            float premise_strength = 1.0f;
            bool all_premises_hold = true;
            
            for (const auto& premise : arg.premises) {
                float strength = query(premise);
                if (strength < 0.3f) {
                    all_premises_hold = false;
                    break;
                }
                premise_strength = std::min(premise_strength, strength);
            }
            
            if (all_premises_hold) {
                // Argument succeeds
                float conclusion_strength = premise_strength * arg.strength;
                
                if (justified.count(arg.conclusion)) {
                    // Take maximum if multiple arguments for same conclusion
                    justified[arg.conclusion] = std::max(justified[arg.conclusion], 
                                                        conclusion_strength);
                } else {
                    justified[arg.conclusion] = conclusion_strength;
                }
            }
        }
        
        return justified;
    }
    
    // Revise beliefs based on new information
    void revise(const std::string& new_info, float strength) {
        // Add new belief
        add_belief(Belief(new_info, strength));
        
        // Recompute all justified beliefs
        auto justified = compute_justified_beliefs();
        
        // Update belief strengths based on justification
        for (const auto& [prop, just_strength] : justified) {
            if (beliefs_.count(prop)) {
                beliefs_[prop].strength = just_strength;
            }
        }
    }
    
private:
    std::unordered_map<std::string, Belief> beliefs_;
    std::vector<Defeater> defeaters_;
    std::vector<Argument> arguments_;
};

} // namespace dnn::reasoning
