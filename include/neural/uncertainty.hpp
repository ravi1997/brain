#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace dnn::neural {

// Uncertainty Quantification using Monte Carlo Dropout
class UncertaintyQuantification {
public:
    UncertaintyQuantification(float dropout_rate = 0.1f) 
        : dropout_rate_(dropout_rate) {}
    
    // Apply dropout to layer activations
    std::vector<float> apply_dropout(const std::vector<float>& activations, 
                                     bool training = true) {
        if (!training || dropout_rate_ == 0.0f) {
            return activations;
        }
        
        std::vector<float> dropped = activations;
        for (auto& val : dropped) {
            if (static_cast<float>(rand()) / RAND_MAX < dropout_rate_) {
                val = 0.0f;
            } else {
                val /= (1.0f - dropout_rate_);  // Scale remaining
            }
        }
        
        return dropped;
    }
    
    // Estimate uncertainty via multiple forward passes
    struct UncertaintyEstimate {
        std::vector<float> mean;
        std::vector<float> variance;
        std::vector<float> epistemic_uncertainty;  // Model uncertainty
        std::vector<float> aleatoric_uncertainty;  // Data uncertainty
    };
    
    template<typename ForwardFunction>
    UncertaintyEstimate estimate(ForwardFunction forward_fn,
                                const std::vector<float>& input,
                                int num_samples = 30) {
        std::vector<std::vector<float>> samples;
        samples.reserve(num_samples);
        
        // Multiple forward passes with dropout
        for (int i = 0; i < num_samples; i++) {
            samples.push_back(forward_fn(input, true));  // dropout=true
        }
        
        UncertaintyEstimate estimate;
        int output_size = samples[0].size();
        
        // Compute mean
        estimate.mean.resize(output_size, 0.0f);
        for (const auto& sample : samples) {
            for (int i = 0; i < output_size; i++) {
                estimate.mean[i] += sample[i];
            }
        }
        for (auto& m : estimate.mean) {
            m /= num_samples;
        }
        
        // Compute variance (epistemic uncertainty)
        estimate.variance.resize(output_size, 0.0f);
        estimate.epistemic_uncertainty.resize(output_size, 0.0f);
        
        for (const auto& sample : samples) {
            for (int i = 0; i < output_size; i++) {
                float diff = sample[i] - estimate.mean[i];
                estimate.variance[i] += diff * diff;
            }
        }
        
        for (int i = 0; i < output_size; i++) {
            estimate.variance[i] /= num_samples;
            estimate.epistemic_uncertainty[i] = std::sqrt(estimate.variance[i]);
        }
        
        // Aleatoric uncertainty (data noise) - simplified
        estimate.aleatoric_uncertainty.resize(output_size, 0.1f);
        
        return estimate;
    }
    
    // Get confidence interval
    std::pair<std::vector<float>, std::vector<float>> 
    confidence_interval(const UncertaintyEstimate& estimate, float z = 1.96f) {
        // z=1.96 for 95% confidence
        std::vector<float> lower(estimate.mean.size());
        std::vector<float> upper(estimate.mean.size());
        
        for (size_t i = 0; i < estimate.mean.size(); i++) {
            float margin = z * estimate.epistemic_uncertainty[i];
            lower[i] = estimate.mean[i] - margin;
            upper[i] = estimate.mean[i] + margin;
        }
        
        return {lower, upper};
    }
    
private:
    float dropout_rate_;
};

} // namespace dnn::neural
