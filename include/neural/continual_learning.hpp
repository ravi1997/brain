#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <cmath>

namespace dnn::neural {

// Elastic Weight Consolidation for continual learning
class ContinualLearning {
public:
    ContinualLearning(int num_params) : num_params_(num_params) {
        weights_.resize(num_params, 0.0f);
        fisher_information_.resize(num_params, 0.0f);
        optimal_weights_.resize(num_params, 0.0f);
    }
    
    // Compute Fisher Information Matrix (diagonal approximation)
    void compute_fisher(const std::vector<std::vector<float>>& gradients) {
        // Fisher = E[∇log p(y|x)^2]
        fisher_information_.assign(num_params_, 0.0f);
        
        for (const auto& grad : gradients) {
            for (int i = 0; i < num_params_ && i < static_cast<int>(grad.size()); i++) {
                fisher_information_[i] += grad[i] * grad[i];
            }
        }
        
        // Average
        if (!gradients.empty()) {
            for (auto& f : fisher_information_) {
                f /= gradients.size();
            }
        }
    }
    
    // Store current weights as optimal for task
    void consolidate() {
        optimal_weights_ = weights_;
    }
    
    // Update weights with EWC penalty
    void update(const std::vector<float>& gradient, float learning_rate = 0.01f, 
               float ewc_lambda = 1000.0f) {
        
        for (int i = 0; i < num_params_; i++) {
            // Standard gradient update
            float update = -learning_rate * gradient[i];
            
            // EWC penalty: prevents changing important weights
            float ewc_penalty = ewc_lambda * fisher_information_[i] * 
                               (weights_[i] - optimal_weights_[i]);
            
            weights_[i] += update - learning_rate * ewc_penalty;
        }
    }
    
    // Get current weights
    std::vector<float> get_weights() const {
        return weights_;
    }
    
   // Set weights
    void set_weights(const std::vector<float>& weights) {
        weights_ = weights;
    }
    
private:
    int num_params_;
    std::vector<float> weights_;
    std::vector<float> fisher_information_;
    std::vector<float> optimal_weights_;
};

} // namespace dnn::neural
