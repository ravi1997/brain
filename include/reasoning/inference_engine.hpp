#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace dnn::reasoning {

// Forward and Backward Chaining Inference Engine
class InferenceEngine {
public:
    struct Rule {
        std::vector<std::string> premises;  // IF these are true
        std::string conclusion;             // THEN this is true
        
        Rule() {}
        Rule(const std::vector<std::string>& p, const std::string& c)
            : premises(p), conclusion(c) {}
    };
    
    InferenceEngine() {}
    
    // Add a rule to knowledge base
    void add_rule(const Rule& rule) {
        rules_.push_back(rule);
    }
    
    // Add a fact
    void add_fact(const std::string& fact) {
        facts_.insert(fact);
    }
    
    // Forward chaining: deduce new facts from known facts
    std::vector<std::string> forward_chain() {
        std::vector<std::string> new_facts;
        bool changed = true;
        
        while (changed) {
            changed = false;
            
            for (const auto& rule : rules_) {
                // Check if all premises are satisfied
                bool all_satisfied = true;
                for (const auto& premise : rule.premises) {
                    if (facts_.find(premise) == facts_.end()) {
                        all_satisfied = false;
                        break;
                    }
                }
                
                // If yes and conclusion not already known, add it
                if (all_satisfied && facts_.find(rule.conclusion) == facts_.end()) {
                    facts_.insert(rule.conclusion);
                    new_facts.push_back(rule.conclusion);
                    changed = true;
                }
            }
        }
        
        return new_facts;
    }
    
    // Backward chaining: prove a goal from known facts
    bool backward_chain(const std::string& goal, 
                       std::unordered_set<std::string>& visited) {
        // Prevent infinite loops
        if (visited.count(goal)) {
            return false;
        }
        visited.insert(goal);
        
        // Base case: goal is a known fact
        if (facts_.count(goal)) {
            return true;
        }
        
        // Recursive case: try to prove via rules
        for (const auto& rule : rules_) {
            if (rule.conclusion == goal) {
                // Try to prove all premises
                bool all_proven = true;
                for (const auto& premise : rule.premises) {
                    if (!backward_chain(premise, visited)) {
                        all_proven = false;
                        break;
                    }
                }
                
                if (all_proven) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    // Public backward chain interface
    bool prove(const std::string& goal) {
        std::unordered_set<std::string> visited;
        return backward_chain(goal, visited);
    }
    
    // Get all known facts
    std::unordered_set<std::string> get_facts() const {
        return facts_;
    }
    
    // Clear all facts
    void clear_facts() {
        facts_.clear();
    }
    
private:
    std::vector<Rule> rules_;
    std::unordered_set<std::string> facts_;
};

} // namespace dnn::reasoning
