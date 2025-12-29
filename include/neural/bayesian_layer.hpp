#pragma once
#include <vector>
#include <random>
#include <cmath>

namespace dnn::neural {

// Bayesian Neural Network Layer with uncertainty quantification
class BayesianLayer {
public:
    BayesianLayer(int input_size, int output_size)
        : input_size_(input_size), output_size_(output_size),
          gen_(std::random_device{}()) {
        
        // Weight mean and variance (Gaussian distribution)
        weight_mean_.resize(input_size * output_size);
        weight_log_var_.resize(input_size * output_size);
        
        // Initialize with small random values
        std::normal_distribution<float> dist(0.0f, 0.1f);
        for (auto& w : weight_mean_) w = dist(gen_);
        for (auto& v : weight_log_var_) v = std::log(0.01f);  // log(0.01) for small variance
    }
    
    // Forward pass with sampling (stochastic)
    std::vector<float> forward(const std::vector<float>& input, bool sample = true) {
        std::vector<float> output(output_size_, 0.0f);
        
        for (int o = 0; o < output_size_; o++) {
            for (int i = 0; i < input_size_ && i < static_cast<int>(input.size()); i++) {
                int idx = o * input_size_ + i;
                
                float weight;
                if (sample) {
                    // Sample from weight distribution: w ~ N(μ, σ²)
                    float std_dev = std::exp(0.5f * weight_log_var_[idx]);
                    std::normal_distribution<float> weight_dist(weight_mean_[idx], std_dev);
                    weight = weight_dist(gen_);
                } else {
                    // Use mean weight
                    weight = weight_mean_[idx];
                }
                
                output[o] += weight * input[i];
            }
        }
        
        return output;
    }
    
    // Forward pass with multiple samples for uncertainty estimation
    std::pair<std::vector<float>, std::vector<float>> forward_with_uncertainty(
        const std::vector<float>& input, int num_samples = 10) {
        
        std::vector<std::vector<float>> samples;
        samples.reserve(num_samples);
        
        for (int s = 0; s < num_samples; s++) {
            samples.push_back(forward(input, true));
        }
        
        // Compute mean and variance
        std::vector<float> mean(output_size_, 0.0f);
        std::vector<float> variance(output_size_, 0.0f);
        
        for (const auto& sample : samples) {
            for (int i = 0; i < output_size_; i++) {
                mean[i] += sample[i];
            }
        }
        
        for (auto& m : mean) m /= num_samples;
        
        for (const auto& sample : samples) {
            for (int i = 0; i < output_size_; i++) {
                float diff = sample[i] - mean[i];
                variance[i] += diff * diff;
            }
        }
        
        for (auto& v : variance) v /= num_samples;
        
        return {mean, variance};
    }
    
private:
    int input_size_;
    int output_size_;
    std::vector<float> weight_mean_;
    std::vector<float> weight_log_var_;
    std::mt19937 gen_;
};

} // namespace dnn::neural
