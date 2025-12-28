#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace dnn::neural {

// Model-Agnostic Meta-Learning (MAML)
class MAML {
public:
    using Parameters = std::vector<float>;
    using GradientFunction = std::function<std::vector<float>(const Parameters&, 
                                                              const std::vector<float>&, 
                                                              const std::vector<float>&)>;
    
    MAML(int param_size, float inner_lr = 0.01f, float outer_lr = 0.001f, int inner_steps = 5)
        : param_size_(param_size), inner_lr_(inner_lr), outer_lr_(outer_lr), 
          inner_steps_(inner_steps) {
        
        // Initialize meta-parameters
        meta_params_.resize(param_size);
        for (auto& p : meta_params_) {
            p = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
        }
    }
    
    // Inner loop: adapt to a specific task
    Parameters adapt(const std::vector<float>& support_x, 
                    const std::vector<float>& support_y,
                    GradientFunction compute_gradient) {
        
        Parameters adapted_params = meta_params_;
        
        // Perform inner loop gradient descent
        for (int step = 0; step < inner_steps_; step++) {
            // Compute gradient on support set
            auto gradient = compute_gradient(adapted_params, support_x, support_y);
            
            // Update adapted parameters
            for (int i = 0; i < param_size_; i++) {
                adapted_params[i] -= inner_lr_ * gradient[i];
            }
        }
        
        return adapted_params;
    }
    
    // Outer loop: meta-update across tasks
    void meta_update(const std::vector<std::pair<std::vector<float>, std::vector<float>>>& tasks,
                    GradientFunction compute_gradient) {
        
        std::vector<float> meta_gradient(param_size_, 0.0f);
        
        // For each task
        for (const auto& [task_x, task_y] : tasks) {
            // Split into support and query sets (simple 50-50 split)
            int split = task_x.size() / 2;
            
            std::vector<float> support_x(task_x.begin(), task_x.begin() + split);
            std::vector<float> support_y(task_y.begin(), task_y.begin() + split);
            std::vector<float> query_x(task_x.begin() + split, task_x.end());
            std::vector<float> query_y(task_y.begin() + split, task_y.end());
            
            // Adapt to task
            auto adapted_params = adapt(support_x, support_y, compute_gradient);
            
            // Compute gradient on query set
            auto task_gradient = compute_gradient(adapted_params, query_x, query_y);
            
            // Accumulate meta-gradient
            for (int i = 0; i < param_size_; i++) {
                meta_gradient[i] += task_gradient[i];
            }
        }
        
        // Average and apply meta-gradient
        for (int i = 0; i < param_size_; i++) {
            meta_gradient[i] /= tasks.size();
            meta_params_[i] -= outer_lr_ * meta_gradient[i];
        }
    }
    
    // Get current meta-parameters
    Parameters get_meta_parameters() const {
        return meta_params_;
    }
    
    // Set meta-parameters
    void set_meta_parameters(const Parameters& params) {
        meta_params_ = params;
    }
    
    // Few-shot learning: quickly adapt to new task
    Parameters few_shot_adapt(const std::vector<float>& few_shot_x,
                             const std::vector<float>& few_shot_y,
                             GradientFunction compute_gradient,
                             int adaptation_steps = -1) {
        
        int steps = adaptation_steps > 0 ? adaptation_steps : inner_steps_;
        
        Parameters adapted = meta_params_;
        for (int step = 0; step < steps; step++) {
            auto gradient = compute_gradient(adapted, few_shot_x, few_shot_y);
            for (int i = 0; i < param_size_; i++) {
                adapted[i] -= inner_lr_ * gradient[i];
            }
        }
        
        return adapted;
    }
    
private:
    int param_size_;
    float inner_lr_;   // Inner loop learning rate
    float outer_lr_;   // Outer loop (meta) learning rate
    int inner_steps_;  // Number of inner loop steps
    Parameters meta_params_;
};

} // namespace dnn::neural
