#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace dnn::neural {

// Mixture of Experts (MoE) with gating network
class MixtureOfExperts {
public:
    struct Expert {
        std::vector<float> weights;
        std::vector<float> bias;
        
        Expert(int input_dim, int output_dim) {
            weights.resize(input_dim * output_dim);
            bias.resize(output_dim);
            
            // Initialize with small random values
            for (auto& w : weights) {
                w = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
            }
            for (auto& b : bias) {
                b = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.01f;
            }
        }
        
        std::vector<float> forward(const std::vector<float>& input) const {
            int input_dim = input.size();
            int output_dim = bias.size();
            std::vector<float> output = bias;
            
            for (int i = 0; i < output_dim; i++) {
                for (int j = 0; j < input_dim; j++) {
                    output[i] += weights[i * input_dim + j] * input[j];
                }
            }
            
            return output;
        }
    };
    
    MixtureOfExperts(int num_experts, int input_dim, int output_dim, int top_k = 2)
        : num_experts_(num_experts), input_dim_(input_dim), 
          output_dim_(output_dim), top_k_(top_k) {
        
        // Create experts
        for (int i = 0; i < num_experts; i++) {
            experts_.emplace_back(input_dim, output_dim);
        }
        
        // Initialize gating network
        gating_weights_.resize(input_dim * num_experts);
        for (auto& w : gating_weights_) {
            w = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
        }
    }
    
    // Forward pass with sparse gating
    std::vector<float> forward(const std::vector<float>& input) {
        // Compute gating scores
        auto gate_logits = compute_gating(input);
        
        // Apply softmax
        auto gate_probs = softmax(gate_logits);
        
        // Select top-k experts
        auto top_experts = select_top_k(gate_probs);
        
        // Normalize top-k gates
        float gate_sum = 0.0f;
        for (auto [idx, prob] : top_experts) {
            gate_sum += prob;
        }
        
        // Weighted combination of expert outputs
        std::vector<float> output(output_dim_, 0.0f);
        
        for (auto [expert_idx, gate_prob] : top_experts) {
            auto expert_output = experts_[expert_idx].forward(input);
            float weight = gate_prob / gate_sum;
            
            for (int i = 0; i < output_dim_; i++) {
                output[i] += weight * expert_output[i];
            }
        }
        
        return output;
    }
    
    // Compute load balancing loss (encourage uniform expert usage)
    float compute_load_balance_loss(const std::vector<std::vector<float>>& batch_inputs) {
        std::vector<float> expert_usage(num_experts_, 0.0f);
        
        for (const auto& input : batch_inputs) {
            auto gate_logits = compute_gating(input);
            auto gate_probs = softmax(gate_logits);
            
            for (int i = 0; i < num_experts_; i++) {
                expert_usage[i] += gate_probs[i];
            }
        }
        
        // Normalize
        for (auto& usage : expert_usage) {
            usage /= batch_inputs.size();
        }
        
        // Compute variance (lower is better for load balancing)
        float mean = 1.0f / num_experts_;
        float variance = 0.0f;
        for (float usage : expert_usage) {
            float diff = usage - mean;
            variance += diff * diff;
        }
        
        return variance;
    }
    
    // Get expert utilization
    std::vector<float> get_expert_usage(const std::vector<std::vector<float>>& batch_inputs) {
        std::vector<float> usage(num_experts_, 0.0f);
        
        for (const auto& input : batch_inputs) {
            auto gate_logits = compute_gating(input);
            auto gate_probs = softmax(gate_logits);
            auto top_experts = select_top_k(gate_probs);
            
            for (auto [idx, _] : top_experts) {
                usage[idx] += 1.0f;
            }
        }
        
        for (auto& u : usage) {
            u /= batch_inputs.size();
        }
        
        return usage;
    }
    
private:
    int num_experts_;
    int input_dim_;
    int output_dim_;
    int top_k_;  // Number of experts to use per input
    
    std::vector<Expert> experts_;
    std::vector<float> gating_weights_;
    
    std::vector<float> compute_gating(const std::vector<float>& input) const {
        std::vector<float> logits(num_experts_, 0.0f);
        
        for (int i = 0; i < num_experts_; i++) {
            for (int j = 0; j < input_dim_ && j < static_cast<int>(input.size()); j++) {
                logits[i] += gating_weights_[i * input_dim_ + j] * input[j];
            }
        }
        
        return logits;
    }
    
    std::vector<float> softmax(const std::vector<float>& logits) const {
        if (logits.empty()) return {};
        
        float max_val = *std::max_element(logits.begin(), logits.end());
        std::vector<float> probs(logits.size());
        float sum = 0.0f;
        
        for (size_t i = 0; i < logits.size(); i++) {
            probs[i] = std::exp(logits[i] - max_val);
            sum += probs[i];
        }
        
        for (auto& p : probs) {
            p /= sum;
        }
        
        return probs;
    }
    
    std::vector<std::pair<int, float>> select_top_k(const std::vector<float>& probs) const {
        std::vector<std::pair<int, float>> indexed_probs;
        indexed_probs.reserve(probs.size());
        
        for (size_t i = 0; i < probs.size(); i++) {
            indexed_probs.emplace_back(i, probs[i]);
        }
        
        // Partial sort to get top-k
        int k = std::min(top_k_, static_cast<int>(indexed_probs.size()));
        std::partial_sort(indexed_probs.begin(),
                         indexed_probs.begin() + k,
                         indexed_probs.end(),
                         [](const auto& a, const auto& b) { return a.second > b.second; });
        
        indexed_probs.resize(k);
        return indexed_probs;
    }
};

} // namespace dnn::neural
