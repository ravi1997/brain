#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>

namespace dnn::neural {

// Gradient-based Meta-Learning (MAML variant with explicit gradient computation)
class GradientMetaLearning {
public:
    using LossFunction = std::function<float(const std::vector<float>&, 
                                             const std::vector<float>&,
                                             const std::vector<float>&)>;
    
    struct Task {
        std::vector<std::vector<float>> support_x;  // Training examples
        std::vector<std::vector<float>> support_y;  // Training labels
        std::vector<std::vector<float>> query_x;    // Test examples
        std::vector<std::vector<float>> query_y;    // Test labels
    };
    
    GradientMetaLearning(int param_size, float meta_lr = 0.001f, float inner_lr = 0.01f)
        : param_size_(param_size), meta_lr_(meta_lr), inner_lr_(inner_lr) {
        
        // Initialize meta-parameters
        meta_params_.resize(param_size);
        for (auto& p : meta_params_) {
            p = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
        }
    }
    
    // Inner loop: adapt to task using gradient descent
    std::vector<float> adapt(const Task& task, int num_steps = 5,
                            LossFunction loss_fn = nullptr) {
        std::vector<float> adapted_params = meta_params_;
        
        if (!loss_fn) {
            loss_fn = [](const std::vector<float>& params,
                        const std::vector<float>& x,
                        const std::vector<float>& y) {
                // Simple MSE loss
                float pred = 0.0f;
                for (size_t i = 0; i < x.size() && i < params.size(); i++) {
                    pred += params[i] * x[i];
                }
                float diff = pred - (y.empty() ? 0.0f : y[0]);
                return diff * diff;
            };
        }
        
        // Gradient descent on support set
        for (int step = 0; step < num_steps; step++) {
            std::vector<float> gradient(param_size_, 0.0f);
            
            // Compute gradient on support set
            for (size_t i = 0; i < task.support_x.size(); i++) {
                auto grad = compute_gradient(adapted_params, task.support_x[i],
                                            task.support_y[i], loss_fn);
                for (size_t j = 0; j < gradient.size(); j++) {
                    gradient[j] += grad[j];
                }
            }
            
            // Normalize
            if (!task.support_x.empty()) {
                for (auto& g : gradient) {
                    g /= task.support_x.size();
                }
            }
            
            // Update adapted parameters
            for (size_t i = 0; i < adapted_params.size(); i++) {
                adapted_params[i] -= inner_lr_ * gradient[i];
            }
        }
        
        return adapted_params;
    }
    
    // Outer loop: meta-update using query set
    void meta_update(const std::vector<Task>& tasks, int inner_steps = 5,
                    LossFunction loss_fn = nullptr) {
        std::vector<float> meta_gradient(param_size_, 0.0f);
        
        for (const auto& task : tasks) {
            // Adapt to task
            auto adapted_params = adapt(task, inner_steps, loss_fn);
            
            // Compute gradient on query set with adapted parameters
            for (size_t i = 0; i < task.query_x.size(); i++) {
                auto grad = compute_gradient(adapted_params, task.query_x[i],
                                            task.query_y[i], loss_fn);
                
                // Accumulate for meta-gradient
                for (size_t j = 0; j < meta_gradient.size(); j++) {
                    meta_gradient[j] += grad[j];
                }
            }
        }
        
        // Normalize
        int total_queries = 0;
        for (const auto& task : tasks) {
            total_queries += task.query_x.size();
        }
        
        if (total_queries > 0) {
            for (auto& g : meta_gradient) {
                g /= total_queries;
            }
        }
        
        // Meta-update
        for (size_t i = 0; i < meta_params_.size(); i++) {
            meta_params_[i] -= meta_lr_ * meta_gradient[i];
        }
    }
    
    // Train meta-learner
    void train(const std::vector<std::vector<Task>>& meta_batches, int epochs = 10) {
        for (int epoch = 0; epoch < epochs; epoch++) {
            for (const auto& batch : meta_batches) {
                meta_update(batch, 5);
            }
        }
    }
    
    // Fine-tune on new task
    std::vector<float> fine_tune(const Task& task, int num_steps = 10,
                                LossFunction loss_fn = nullptr) {
        return adapt(task, num_steps, loss_fn);
    }
    
    // Evaluate on task
    float evaluate(const Task& task, const std::vector<float>& params,
                  LossFunction loss_fn = nullptr) {
        if (!loss_fn) {
            loss_fn = [](const std::vector<float>& p,
                        const std::vector<float>& x,
                        const std::vector<float>& y) {
                float pred = 0.0f;
                for (size_t i = 0; i < x.size() && i < p.size(); i++) {
                    pred += p[i] * x[i];
                }
                float diff = pred - (y.empty() ? 0.0f : y[0]);
                return diff * diff;
            };
        }
        
        float total_loss = 0.0f;
        
        for (size_t i = 0; i < task.query_x.size(); i++) {
            total_loss += loss_fn(params, task.query_x[i], task.query_y[i]);
        }
        
        return task.query_x.empty() ? 0.0f : total_loss / task.query_x.size();
    }
    
    // Get meta-parameters
    std::vector<float> get_meta_params() const {
        return meta_params_;
    }
    
    // Set meta-parameters
    void set_meta_params(const std::vector<float>& params) {
        meta_params_ = params;
    }
    
private:
    int param_size_;
    float meta_lr_;
    float inner_lr_;
    std::vector<float> meta_params_;
    
    // Compute gradient using finite differences
    std::vector<float> compute_gradient(const std::vector<float>& params,
                                       const std::vector<float>& x,
                                       const std::vector<float>& y,
                                       LossFunction loss_fn) {
        std::vector<float> gradient(params.size());
        float epsilon = 1e-4f;
        
        float base_loss = loss_fn(params, x, y);
        
        for (size_t i = 0; i < params.size(); i++) {
            std::vector<float> params_plus = params;
            params_plus[i] += epsilon;
            
            float loss_plus = loss_fn(params_plus, x, y);
            gradient[i] = (loss_plus - base_loss) / epsilon;
        }
        
        return gradient;
    }
};

} // namespace dnn::neural
