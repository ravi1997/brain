#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace dnn::neural {

// Capsule Networks for hierarchical representations
class CapsuleNetwork {
public:
    struct Capsule {
        std::vector<float> activation;  // Capsule output
        float probability;              // Routing weight
        
        Capsule(int dim = 8) : activation(dim, 0.0f), probability(0.0f) {}
    };
    
    CapsuleNetwork(int num_capsules = 32, int capsule_dim = 8, int routing_iterations = 3)
        : num_capsules_(num_capsules), capsule_dim_(capsule_dim), 
          routing_iterations_(routing_iterations) {
        capsules_.resize(num_capsules, Capsule(capsule_dim));
    }
    
    // Squash function (non-linear activation for capsules)
    std::vector<float> squash(const std::vector<float>& input) const {
        float norm_sq = 0.0f;
        for (float val : input) {
            norm_sq += val * val;
        }
        
        float norm = std::sqrt(norm_sq);
        float scale = norm_sq / (1.0f + norm_sq);
        
        std::vector<float> output(input.size());
        for (size_t i = 0; i < input.size(); i++) {
            output[i] = scale * input[i] / (norm + 1e-7f);
        }
        
        return output;
    }
    
    // Dynamic routing algorithm
    std::vector<Capsule> dynamic_routing(const std::vector<std::vector<float>>& predictions) {
        int num_input = predictions.size();
        int num_output = num_capsules_;
        
        // Initialize routing logits
        std::vector<std::vector<float>> b(num_input, std::vector<float>(num_output, 0.0f));
        
        std::vector<Capsule> output_capsules(num_output, Capsule(capsule_dim_));
        
        // Routing iterations
        for (int iter = 0; iter < routing_iterations_; iter++) {
            // Softmax over routing logits
            std::vector<std::vector<float>> c(num_input, std::vector<float>(num_output));
            for (int i = 0; i < num_input; i++) {
                c[i] = softmax(b[i]);
            }
            
            // Weighted sum of predictions
            for (int j = 0; j < num_output; j++) {
                std::vector<float> s(capsule_dim_, 0.0f);
                
                for (int i = 0; i < num_input && i < static_cast<int>(predictions.size()); i++) {
                    for (int d = 0; d < capsule_dim_ && d < static_cast<int>(predictions[i].size()); d++) {
                        s[d] += c[i][j] * predictions[i][d];
                    }
                }
                
                // Apply squash
                output_capsules[j].activation = squash(s);
                output_capsules[j].probability = vector_norm(output_capsules[j].activation);
                
                // Update routing logits (except last iteration)
                if (iter < routing_iterations_ - 1) {
                    for (int i = 0; i < num_input && i < static_cast<int>(predictions.size()); i++) {
                        float agreement = dot_product(predictions[i], output_capsules[j].activation);
                        b[i][j] += agreement;
                    }
                }
            }
        }
        
        return output_capsules;
    }
    
    // Forward pass
    std::vector<float> forward(const std::vector<float>& input) {
        // Simple transformation to predictions
        std::vector<std::vector<float>> predictions;
        predictions.reserve(num_capsules_);
        
        for (int i = 0; i < num_capsules_; i++) {
            std::vector<float> pred(capsule_dim_);
            for (int d = 0; d < capsule_dim_ && (i * capsule_dim_ + d) < static_cast<int>(input.size()); d++) {
                pred[d] = input[i * capsule_dim_ + d];
            }
            predictions.push_back(pred);
        }
        
        // Apply dynamic routing
        auto output_caps = dynamic_routing(predictions);
        
        // Flatten output
        std::vector<float> output;
        output.reserve(num_capsules_ * capsule_dim_);
        for (const auto& cap : output_caps) {
            output.insert(output.end(), cap.activation.begin(), cap.activation.end());
        }
        
        return output;
    }
    
    // Get capsule activations
    std::vector<Capsule> get_capsules() const {
        return capsules_;
    }
    
private:
    int num_capsules_;
    int capsule_dim_;
    int routing_iterations_;
    std::vector<Capsule> capsules_;
    
    std::vector<float> softmax(const std::vector<float>& input) const {
        if (input.empty()) return {};
        
        float max_val = *std::max_element(input.begin(), input.end());
        std::vector<float> exp_vals(input.size());
        float sum = 0.0f;
        
        for (size_t i = 0; i < input.size(); i++) {
            exp_vals[i] = std::exp(input[i] - max_val);
            sum += exp_vals[i];
        }
        
        for (auto& val : exp_vals) {
            val /= sum;
        }
        
        return exp_vals;
    }
    
    float dot_product(const std::vector<float>& a, const std::vector<float>& b) const {
        float sum = 0.0f;
        for (size_t i = 0; i < std::min(a.size(), b.size()); i++) {
            sum += a[i] * b[i];
        }
        return sum;
    }
    
    float vector_norm(const std::vector<float>& v) const {
        float sum = 0.0f;
        for (float val : v) {
            sum += val * val;
        }
        return std::sqrt(sum);
    }
};

} // namespace dnn::neural
