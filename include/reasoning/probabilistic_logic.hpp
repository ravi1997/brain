#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>

namespace dnn::reasoning {

// Probabilistic Logical Inference with Bayesian Networks
class ProbabilisticLogic {
public:
    struct Variable {
        std::string name;
        std::vector<std::string> domain;  // Possible values
        int current_value_idx;
        
        Variable() : current_value_idx(0) {}
        Variable(const std::string& n, const std::vector<std::string>& d)
            : name(n), domain(d), current_value_idx(0) {}
    };
    
    struct ConditionalProbability {
        std::string variable;
        std::vector<std::string> parents;
        std::unordered_map<std::string, float> probabilities;  // P(variable=val | parents=config)
        
        ConditionalProbability() {}
    };
    
    ProbabilisticLogic() {}
    
    // Add variable
    void add_variable(const Variable& var) {
        variables_[var.name] = var;
    }
    
    // Add conditional probability table
    void add_cpt(const std::string& variable,
                 const std::vector<std::string>& parents,
                 const std::unordered_map<std::string, float>& probs) {
        ConditionalProbability cpt;
        cpt.variable = variable;
        cpt.parents = parents;
        cpt.probabilities = probs;
        cpts_[variable] = cpt;
    }
    
    // Compute probability of evidence
    float compute_probability(const std::unordered_map<std::string, std::string>& assignment) {
        float prob = 1.0f;
        
        for (const auto& [var_name, value] : assignment) {
            if (cpts_.count(var_name)) {
                const auto& cpt = cpts_[var_name];
                
                // Build key for CPT lookup
                std::string key = value;
                for (const auto& parent : cpt.parents) {
                    if (assignment.count(parent)) {
                        key += "|" + assignment.at(parent);
                    }
                }
                
                if (cpt.probabilities.count(key)) {
                    prob *= cpt.probabilities.at(key);
                } else {
                    // Default uniform probability
                    prob *= 0.5f;
                }
            }
        }
        
        return prob;
    }
    
    // Inference by enumeration (exact)
    float infer(const std::string& query_var, const std::string& query_value,
               const std::unordered_map<std::string, std::string>& evidence) {
        // Enumerate all possible assignments
        std::vector<std::string> hidden_vars;
        
        for (const auto& [name, var] : variables_) {
            if (name != query_var && evidence.find(name) == evidence.end()) {
                hidden_vars.push_back(name);
            }
        }
        
        // Sum over all hidden variable assignments
        float total_prob = 0.0f;
        enumerate_assignments(hidden_vars, 0, evidence, query_var, query_value, total_prob);
        
        // Normalize
        float norm_constant = 0.0f;
        for (const auto& value : variables_[query_var].domain) {
            float prob = 0.0f;
            enumerate_assignments(hidden_vars, 0, evidence, query_var, value, prob);
            norm_constant += prob;
        }
        
        return norm_constant > 0 ? total_prob / norm_constant : 0.0f;
    }
    
    // Sampling-based inference (approximate)
    float infer_sampling(const std::string& query_var, const std::string& query_value,
                        const std::unordered_map<std::string, std::string>& evidence,
                        int num_samples = 1000) {
        int count = 0;
        int total = 0;
        
        for (int i = 0; i < num_samples; i++) {
            auto sample = forward_sample();
            
            // Check if sample matches evidence
            bool matches_evidence = true;
            for (const auto& [var, val] : evidence) {
                if (sample[var] != val) {
                    matches_evidence = false;
                    break;
                }
            }
            
            if (matches_evidence) {
                total++;
                if (sample[query_var] == query_value) {
                    count++;
                }
            }
        }
        
        return total > 0 ? static_cast<float>(count) / total : 0.0f;
    }
    
    // Maximum a posteriori (MAP) inference
    std::unordered_map<std::string, std::string> 
    map_inference(const std::unordered_map<std::string, std::string>& evidence) {
        std::unordered_map<std::string, std::string> best_assignment = evidence;
        float best_prob = 0.0f;
        
        // Simple greedy approach for hidden variables
        for (const auto& [name, var] : variables_) {
            if (evidence.find(name) == evidence.end()) {
                float max_prob = 0.0f;
                std::string best_value;
                
                for (const auto& value : var.domain) {
                    best_assignment[name] = value;
                    float prob = compute_probability(best_assignment);
                    
                    if (prob > max_prob) {
                        max_prob = prob;
                        best_value = value;
                    }
                }
                
                best_assignment[name] = best_value;
            }
        }
        
        return best_assignment;
    }
    
private:
    std::unordered_map<std::string, Variable> variables_;
    std::unordered_map<std::string, ConditionalProbability> cpts_;
    
    void enumerate_assignments(const std::vector<std::string>& vars, int idx,
                               std::unordered_map<std::string, std::string> assignment,
                               const std::string& query_var, const std::string& query_value,
                               float& total_prob) {
        if (idx >= static_cast<int>(vars.size())) {
            // Complete assignment - compute probability
            assignment[query_var] = query_value;
            total_prob += compute_probability(assignment);
            return;
        }
        
        // Try each value for current variable
        const auto& var = variables_[vars[idx]];
        for (const auto& value : var.domain) {
            assignment[vars[idx]] = value;
            enumerate_assignments(vars, idx + 1, assignment, query_var, query_value, total_prob);
        }
    }
    
    std::unordered_map<std::string, std::string> forward_sample() {
        std::unordered_map<std::string, std::string> sample;
        
        // Sample in topological order (simplified: assume order in variables_)
        for (const auto& [name, var] : variables_) {
            if (cpts_.count(name)) {
                const auto& cpt = cpts_[name];
                
                // Build parent configuration
                std::string parent_config;
                for (const auto& parent : cpt.parents) {
                    if (sample.count(parent)) {
                        parent_config += "|" + sample[parent];
                    }
                }
                
                // Sample from conditional distribution
                float r = static_cast<float>(rand()) / RAND_MAX;
                float cumulative = 0.0f;
                
                for (const auto& value : var.domain) {
                    std::string key = value + parent_config;
                    if (cpt.probabilities.count(key)) {
                        cumulative += cpt.probabilities.at(key);
                        if (r <= cumulative) {
                            sample[name] = value;
                            break;
                        }
                    }
                }
                
                if (sample.find(name) == sample.end()) {
                    sample[name] = var.domain[0];  // Fallback
                }
            } else {
                // Prior probability (uniform)
                int idx = rand() % var.domain.size();
                sample[name] = var.domain[idx];
            }
        }
        
        return sample;
    }
};

} // namespace dnn::reasoning
