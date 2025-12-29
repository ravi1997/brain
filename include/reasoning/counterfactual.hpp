#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <cmath>

namespace dnn::reasoning {

// Counterfactual Reasoning - what would have happened if...
class CounterfactualReasoning {
public:
    struct FactualWorld {
        std::unordered_map<std::string, float> variables;
        std::unordered_map<std::string, std::function<float(const std::unordered_map<std::string, float>&)>> mechanisms;
    };
    
    struct CounterfactualQuery {
        std::string variable;  // What if this was different?
        float counterfactual_value;
        std::string target;    // What would happen to this?
    };
    
    CounterfactualReasoning() {}
    
    // Compute counterfactual: Y_{X=x}(u) 
    // "What would Y be if X were x, given actual world u?"
    float compute_counterfactual(const CounterfactualQuery& query,
                                 const FactualWorld& factual) {
        // Step 1: Abduction - infer unobserved variables (exogenous noise)
        auto exogenous = abduction(factual);
        
        // Step 2: Action - set X = x (intervention)
        FactualWorld counterfactual = factual;
        counterfactual.variables[query.variable] = query.counterfactual_value;
        
        // Step 3: Prediction - compute Y in counterfactual world
        return prediction(counterfactual, query.target, exogenous);
    }
    
    // Probability of necessity: P(Y_{X=0}=0 | X=1, Y=1)
    // "Was X necessary for Y?"
    float probability_of_necessity(const std::string& cause,
                                  const std::string& effect,
                                  const FactualWorld& factual,
                                  int num_samples = 100) {
        int count_necessary = 0;
        int count_total = 0;
        
        for (int i = 0; i < num_samples; i++) {
            // Check if in factual world: X=1 and Y=1
            if (factual.variables.count(cause) && factual.variables.count(effect)) {
                float x_factual = factual.variables.at(cause);
                float y_factual = factual.variables.at(effect);
                
                if (x_factual > 0.5f && y_factual > 0.5f) {
                    // Compute counterfactual: what if X=0?
                    CounterfactualQuery query;
                    query.variable = cause;
                    query.counterfactual_value = 0.0f;
                    query.target = effect;
                    
                    float y_counterfactual = compute_counterfactual(query, factual);
                    
                    // Was X necessary? (Y would be 0 if X were 0)
                    if (y_counterfactual < 0.5f) {
                        count_necessary++;
                    }
                    count_total++;
                }
            }
        }
        
        return count_total > 0 ? static_cast<float>(count_necessary) / count_total : 0.0f;
    }
    
    // Probability of sufficiency: P(Y_{X=1}=1 | X=0, Y=0)
    // "Was X sufficient for Y?"
    float probability_of_sufficiency(const std::string& cause,
                                    const std::string& effect,
                                    const FactualWorld& factual,
                                    int num_samples = 100) {
        int count_sufficient = 0;
        int count_total = 0;
        
        for (int i = 0; i < num_samples; i++) {
            // Check if in factual world: X=0 and Y=0
            if (factual.variables.count(cause) && factual.variables.count(effect)) {
                float x_factual = factual.variables.at(cause);
                float y_factual = factual.variables.at(effect);
                
                if (x_factual < 0.5f && y_factual < 0.5f) {
                    // Compute counterfactual: what if X=1?
                    CounterfactualQuery query;
                    query.variable = cause;
                    query.counterfactual_value = 1.0f;
                    query.target = effect;
                    
                    float y_counterfactual = compute_counterfactual(query, factual);
                    
                    // Was X sufficient? (Y would be 1 if X were 1)
                    if (y_counterfactual > 0.5f) {
                        count_sufficient++;
                    }
                    count_total++;
                }
            }
        }
        
        return count_total > 0 ? static_cast<float>(count_sufficient) / count_total : 0.0f;
    }
    
    // Explain outcome via counterfactuals
    std::string explain(const std::string& variable,
                       const FactualWorld& factual) {
        std::string explanation;
        
        // Find what could have changed the outcome
        for (const auto& [var_name, var_value] : factual.variables) {
            if (var_name == variable) continue;
            
            // What if this variable was different?
            CounterfactualQuery query;
            query.variable = var_name;
            query.counterfactual_value = 1.0f - var_value;  // Flip
            query.target = variable;
            
            float counterfactual_target = compute_counterfactual(query, factual);
            float factual_target = factual.variables.count(variable) ? 
                                  factual.variables.at(variable) : 0.0f;
            
            // Significant change?
            if (std::abs(counterfactual_target - factual_target) > 0.3f) {
                explanation += var_name + " caused " + variable + ". ";
            }
        }
        
        return explanation.empty() ? "No clear cause found" : explanation;
    }
    
private:
    // Abduction: infer exogenous variables from observations
    std::unordered_map<std::string, float> abduction(const FactualWorld& factual) {
        std::unordered_map<std::string, float> exogenous;
        
        // Simplified: assume exogenous noise is small
        for (const auto& [name, value] : factual.variables) {
            exogenous["U_" + name] = 0.0f;
        }
        
        return exogenous;
    }
    
    // Prediction: compute outcome in (possibly counterfactual) world
    float prediction(const FactualWorld& world,
                    const std::string& target,
                    const std::unordered_map<std::string, float>& exogenous) {
        auto values = world.variables;
        
        // Apply causal mechanisms
        if (world.mechanisms.count(target)) {
            return world.mechanisms.at(target)(values);
        }
        
        return values.count(target) ? values.at(target) : 0.0f;
    }
};

} // namespace dnn::reasoning
