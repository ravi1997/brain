#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <memory>

namespace dnn::neural {

// Differentiable Reasoning Modules - learnable logic operations
class DifferentiableReasoning {
public:
    struct LogicGate {
        std::vector<float> weights;
        float bias;
        std::string type;  // "and", "or", "not", "implies"
        
        LogicGate() : bias(0) {}
        LogicGate(const std::string& t, int num_inputs)
            : bias(0), type(t) {
            weights.resize(num_inputs, 1.0f / num_inputs);
        }
    };
    
    DifferentiableReasoning() {}
    
    // Differentiable AND (product t-norm)
    float fuzzy_and(const std::vector<float>& inputs) {
        float result = 1.0f;
        for (float val : inputs) {
            result *= sigmoid(val);
        }
        return result;
    }
    
    // Differentiable OR (probabilistic sum)
    float fuzzy_or(const std::vector<float>& inputs) {
        float result = 0.0f;
        for (float val : inputs) {
            result = result + sigmoid(val) - result * sigmoid(val);
        }
        return result;
    }
    
    // Differentiable NOT
    float fuzzy_not(float input) {
        return 1.0f - sigmoid(input);
    }
    
    // Differentiable IMPLIES (a -> b = ¬a ∨ b)
    float fuzzy_implies(float a, float b) {
        return fuzzy_or({fuzzy_not(a), b});
    }
    
    // Forward pass through logic gate
    float forward(const LogicGate& gate, const std::vector<float>& inputs) {
        float weighted_sum = gate.bias;
        
        for (size_t i = 0; i < inputs.size() && i < gate.weights.size(); i++) {
            weighted_sum += gate.weights[i] * inputs[i];
        }
        
        if (gate.type == "and") {
            return fuzzy_and(inputs);
        } else if (gate.type == "or") {
            return fuzzy_or(inputs);
        } else if (gate.type == "not") {
            return fuzzy_not(inputs.empty() ? 0.0f : inputs[0]);
        } else if (gate.type == "implies") {
            if (inputs.size() >= 2) {
                return fuzzy_implies(inputs[0], inputs[1]);
            }
        }
        
        return sigmoid(weighted_sum);
    }
    
    // Backward pass - compute gradients
    std::vector<float> backward(const LogicGate& gate, 
                                const std::vector<float>& inputs,
                                float grad_output) {
        std::vector<float> grad_inputs(inputs.size(), 0.0f);
        
        // Compute gradient using chain rule
        for (size_t i = 0; i < inputs.size(); i++) {
            float epsilon = 0.0001f;
            std::vector<float> inputs_plus = inputs;
            inputs_plus[i] += epsilon;
            
            float output_plus = forward(gate, inputs_plus);
            float output = forward(gate, inputs);
            
            float local_grad = (output_plus - output) / epsilon;
            grad_inputs[i] = grad_output * local_grad;
        }
        
        return grad_inputs;
    }
    
    // Chain multiple reasoning steps
    float reason(const std::vector<LogicGate>& gates,
                const std::unordered_map<std::string, float>& facts) {
        std::unordered_map<std::string, float> values = facts;
        
        // Execute gates in sequence
        for (size_t i = 0; i < gates.size(); i++) {
            std::vector<float> inputs;
            
            // Collect inputs from previous results or facts
            for (const auto& [fact_name, fact_value] : values) {
                inputs.push_back(fact_value);
            }
            
            float result = forward(gates[i], inputs);
            values["gate" + std::to_string(i)] = result;
        }
        
        return values.empty() ? 0.0f : values.begin()->second;
    }
    
private:
    float sigmoid(float x) const {
        return 1.0f / (1.0f + std::exp(-x));
    }
};

} // namespace dnn::neural
