#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>

namespace dnn::neural {

// Adversarial Robustness Training
class AdversarialRobustness {
public:
    enum class AttackType {
        FGSM,           // Fast Gradient Sign Method
        PGD,            // Projected Gradient Descent
        DEEPFOOL,       // DeepFool
        CARLINI_WAGNER  // C&W
    };
    
    struct AdversarialExample {
        std::vector<float> perturbed_input;
        std::vector<float> perturbation;
        float perturbation_magnitude;
        std::string original_label;
        std::string adversarial_label;
        
        AdversarialExample() : perturbation_magnitude(0) {}
    };
    
    AdversarialRobustness(float epsilon = 0.3f) : epsilon_(epsilon) {}
    
    // Generate adversarial example using FGSM
    AdversarialExample fgsm(const std::vector<float>& input,
                           const std::vector<float>& true_label,
                           std::function<std::vector<float>(const std::vector<float>&)> model) {
        AdversarialExample adv;
        adv.perturbed_input = input;
        adv.perturbation.resize(input.size(), 0.0f);
        
        // Compute gradient of loss w.r.t. input
        auto gradient = compute_gradient(input, true_label, model);
        
        // Add perturbation in direction of sign of gradient
        for (size_t i = 0; i < input.size(); i++) {
            float sign = (gradient[i] > 0) ? 1.0f : -1.0f;
            adv.perturbation[i] = epsilon_ * sign;
            adv.perturbed_input[i] = input[i] + adv.perturbation[i];
            
            // Clip to valid range [0, 1]
            adv.perturbed_input[i] = std::max(0.0f, std::min(1.0f, adv.perturbed_input[i]));
        }
        
        adv.perturbation_magnitude = compute_magnitude(adv.perturbation);
        
        return adv;
    }
    
    // Generate adversarial example using PGD (iterative FGSM)
    AdversarialExample pgd(const std::vector<float>& input,
                          const std::vector<float>& true_label,
                          std::function<std::vector<float>(const std::vector<float>&)> model,
                          int iterations = 10,
                          float step_size = 0.01f) {
        AdversarialExample adv;
        adv.perturbed_input = input;
        adv.perturbation.resize(input.size(), 0.0f);
        
        // Start with random perturbation
        for (size_t i = 0; i < input.size(); i++) {
            adv.perturbation[i] = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2 * epsilon_;
            adv.perturbed_input[i] = input[i] + adv.perturbation[i];
        }
        
        // Iterative gradient ascent
        for (int iter = 0; iter < iterations; iter++) {
            auto gradient = compute_gradient(adv.perturbed_input, true_label, model);
            
            // Update perturbation
            for (size_t i = 0; i < input.size(); i++) {
                float sign = (gradient[i] > 0) ? 1.0f : -1.0f;
                adv.perturbation[i] += step_size * sign;
                
                // Project to epsilon ball
                adv.perturbation[i] = std::max(-epsilon_, std::min(epsilon_, adv.perturbation[i]));
                
                // Compute perturbed input
                adv.perturbed_input[i] = input[i] + adv.perturbation[i];
                adv.perturbed_input[i] = std::max(0.0f, std::min(1.0f, adv.perturbed_input[i]));
            }
        }
        
        adv.perturbation_magnitude = compute_magnitude(adv.perturbation);
        
        return adv;
    }
    
    // Adversarial training: train on mixture of clean and adversarial examples
    std::vector<std::pair<std::vector<float>, std::vector<float>>> 
    adversarial_training_batch(
        const std::vector<std::pair<std::vector<float>, std::vector<float>>>& clean_batch,
        std::function<std::vector<float>(const std::vector<float>&)> model,
        float adversarial_ratio = 0.5f) {
        
        std::vector<std::pair<std::vector<float>, std::vector<float>>> augmented_batch;
        
        for (const auto& [input, label] : clean_batch) {
            // Always add clean example
            augmented_batch.emplace_back(input, label);
            
            // Add adversarial example with probability
            if (static_cast<float>(rand()) / RAND_MAX < adversarial_ratio) {
                auto adv = fgsm(input, label, model);
                augmented_batch.emplace_back(adv.perturbed_input, label);
            }
        }
        
        return augmented_batch;
    }
    
    // Evaluate robustness
    float evaluate_robustness(
        const std::vector<std::pair<std::vector<float>, std::vector<float>>>& test_set,
        std::function<std::vector<float>(const std::vector<float>&)> model,
        AttackType attack = AttackType::FGSM) {
        
        int correct_clean = 0;
        int correct_adversarial = 0;
        
        for (const auto& [input, true_label] : test_set) {
            // Test on clean input
            auto pred_clean = model(input);
            if (argmax(pred_clean) == argmax(true_label)) {
                correct_clean++;
            }
            
            // Test on adversarial input
            AdversarialExample adv;
            
            switch (attack) {
                case AttackType::FGSM:
                    adv = fgsm(input, true_label, model);
                    break;
                case AttackType::PGD:
                    adv = pgd(input, true_label, model);
                    break;
                default:
                    adv = fgsm(input, true_label, model);
            }
            
            auto pred_adv = model(adv.perturbed_input);
            if (argmax(pred_adv) == argmax(true_label)) {
                correct_adversarial++;
            }
        }
        
        float robust_accuracy = static_cast<float>(correct_adversarial) / test_set.size();
        return robust_accuracy;
    }
    
    // Certified defense: randomized smoothing
    std::vector<float> randomized_smoothing(
        const std::vector<float>& input,
        std::function<std::vector<float>(const std::vector<float>&)> model,
        int num_samples = 100,
        float noise_std = 0.1f) {
        
        std::vector<float> average_output;
        
        for (int i = 0; i < num_samples; i++) {
            // Add Gaussian noise
            std::vector<float> noisy_input = input;
            
            for (auto& val : noisy_input) {
                float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2 * noise_std;
                val += noise;
                val = std::max(0.0f, std::min(1.0f, val));
            }
            
            // Get prediction
            auto output = model(noisy_input);
            
            // Accumulate
            if (average_output.empty()) {
                average_output = output;
            } else {
                for (size_t j = 0; j < output.size(); j++) {
                    average_output[j] += output[j];
                }
            }
        }
        
        // Average
        for (auto& val : average_output) {
            val /= num_samples;
        }
        
        return average_output;
    }
    
private:
    float epsilon_;
    
    std::vector<float> compute_gradient(
        const std::vector<float>& input,
        const std::vector<float>& true_label,
        std::function<std::vector<float>(const std::vector<float>&)> model) {
        
        std::vector<float> gradient(input.size());
        float epsilon = 0.001f;
        
        // Compute gradient using finite differences
        for (size_t i = 0; i < input.size(); i++) {
            std::vector<float> input_plus = input;
            input_plus[i] += epsilon;
            
            auto output = model(input);
            auto output_plus = model(input_plus);
            
            // Cross-entropy gradient approximation
            float loss = cross_entropy_loss(output, true_label);
            float loss_plus = cross_entropy_loss(output_plus, true_label);
            
            gradient[i] = (loss_plus - loss) / epsilon;
        }
        
        return gradient;
    }
    
    float cross_entropy_loss(const std::vector<float>& predicted,
                            const std::vector<float>& true_label) {
        float loss = 0.0f;
        
        for (size_t i = 0; i < std::min(predicted.size(), true_label.size()); i++) {
            if (true_label[i] > 0) {
                loss -= true_label[i] * std::log(std::max(predicted[i], 1e-7f));
            }
        }
        
        return loss;
    }
    
    float compute_magnitude(const std::vector<float>& vec) {
        float sum = 0.0f;
        for (float v : vec) {
            sum += v * v;
        }
        return std::sqrt(sum);
    }
    
    int argmax(const std::vector<float>& vec) {
        return std::max_element(vec.begin(), vec.end()) - vec.begin();
    }
};

} // namespace dnn::neural
