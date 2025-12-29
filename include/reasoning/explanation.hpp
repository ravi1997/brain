#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace dnn::reasoning {

// Explanation Generation for model interpretability
class ExplanationGeneration {
public:
    struct Feature {
        std::string name;
        float value;
        float importance;  // How important for decision
        
        Feature() : value(0), importance(0) {}
        Feature(const std::string& n, float v, float imp = 0.0f)
            : name(n), value(v), importance(imp) {}
    };
    
    struct Decision {
        std::string prediction;
        float confidence;
        std::vector<Feature> input_features;
        
        Decision() : confidence(0) {}
    };
    
    struct Explanation {
        std::string natural_language;
        std::vector<Feature> key_features;  // Most important features
        std::vector<std::string> rules;     // Extracted rules
        std::unordered_map<std::string, float> counterfactuals;  // What-if scenarios
        
        Explanation() {}
    };
    
    ExplanationGeneration() {}
    
    // Generate explanation for a decision
    Explanation explain(const Decision& decision) {
        Explanation exp;
        
        // Extract key features (highest importance)
        exp.key_features = extract_key_features(decision.input_features, 5);
        
        // Generate natural language explanation
        exp.natural_language = generate_text_explanation(decision, exp.key_features);
        
        // Extract rules
        exp.rules = extract_rules(decision, exp.key_features);
        
        // Generate counterfactuals
        exp.counterfactuals = generate_counterfactuals(decision);
        
        return exp;
    }
    
    // LIME-like local explanation
    Explanation lime_explain(const Decision& decision,
                            std::function<std::string(const std::vector<Feature>&)> model,
                            int num_samples = 100) {
        Explanation exp;
        
        // Sample perturbations around the instance
        std::vector<std::vector<Feature>> samples;
        std::vector<std::string> predictions;
        std::vector<float> weights;  // Based on distance to original
        
        for (int i = 0; i < num_samples; i++) {
            auto perturbed = perturb_features(decision.input_features);
            samples.push_back(perturbed);
            predictions.push_back(model(perturbed));
            
            float distance = compute_distance(decision.input_features, perturbed);
            weights.push_back(std::exp(-distance));  // Kernel weight
        }
        
        // Fit linear model to approximate behavior locally
        auto importances = fit_linear_model(samples, predictions, 
                                           weights, decision.prediction);
        
        // Create key features with importances
        for (size_t i = 0; i < decision.input_features.size(); i++) {
            Feature f = decision.input_features[i];
            if (i < importances.size()) {
                f.importance = importances[i];
            }
            exp.key_features.push_back(f);
        }
        
        // Sort by importance
        std::sort(exp.key_features.begin(), exp.key_features.end(),
                 [](const Feature& a, const Feature& b) {
                     return std::abs(a.importance) > std::abs(b.importance);
                 });
        
        // Keep top features
        if (exp.key_features.size() > 5) {
            exp.key_features.resize(5);
        }
        
        exp.natural_language = generate_text_explanation(decision, exp.key_features);
        
        return exp;
    }
    
    // SHAP-like explanation (simplified)
    std::vector<float> shap_values(const Decision& decision,
                                  std::function<std::string(const std::vector<Feature>&)> model) {
        std::vector<float> shapley_values(decision.input_features.size(), 0.0f);
        
        // Compute marginal contributions
        for (size_t i = 0; i < decision.input_features.size(); i++) {
            // With feature i
            float pred_with = score_prediction(model(decision.input_features), 
                                              decision.prediction);
            
            // Without feature i (set to baseline)
            auto without_i = decision.input_features;
            without_i[i].value = 0.0f;  // Baseline value
            float pred_without = score_prediction(model(without_i), 
                                                 decision.prediction);
            
            shapley_values[i] = pred_with - pred_without;
        }
        
        return shapley_values;
    }
    
private:
    std::vector<Feature> extract_key_features(const std::vector<Feature>& features, 
                                              int top_k) {
        auto sorted = features;
        std::sort(sorted.begin(), sorted.end(),
                 [](const Feature& a, const Feature& b) {
                     return std::abs(a.importance) > std::abs(b.importance);
                 });
        
        if (sorted.size() > static_cast<size_t>(top_k)) {
            sorted.resize(top_k);
        }
        
        return sorted;
    }
    
    std::string generate_text_explanation(const Decision& decision,
                                         const std::vector<Feature>& key_features) {
        std::string exp = "The model predicted '" + decision.prediction + 
                         "' with " + std::to_string(int(decision.confidence * 100)) + 
                         "% confidence. ";
        
        exp += "Key factors: ";
        
        for (size_t i = 0; i < key_features.size(); i++) {
            const auto& f = key_features[i];
            exp += f.name + "=" + std::to_string(f.value);
            
            if (f.importance > 0) {
                exp += " (positive influence)";
            } else if (f.importance < 0) {
                exp += " (negative influence)";
            }
            
            if (i < key_features.size() - 1) {
                exp += ", ";
            }
        }
        
        return exp;
    }
    
    std::vector<std::string> extract_rules(const Decision& decision,
                                          const std::vector<Feature>& key_features) {
        std::vector<std::string> rules;
        
        for (const auto& f : key_features) {
            if (std::abs(f.importance) > 0.3f) {
                std::string rule = "IF " + f.name;
                
                if (f.importance > 0 && f.value > 0.5f) {
                    rule += " is high THEN predict " + decision.prediction;
                } else if (f.importance < 0 && f.value < 0.5f) {
                    rule += " is low THEN predict " + decision.prediction;
                }
                
                rules.push_back(rule);
            }
        }
        
        return rules;
    }
    
    std::unordered_map<std::string, float> generate_counterfactuals(const Decision& decision) {
        std::unordered_map<std::string, float> counterfactuals;
        
        for (const auto& f : decision.input_features) {
            if (std::abs(f.importance) > 0.2f) {
                float alternative = (f.value > 0.5f) ? 0.0f : 1.0f;
                counterfactuals["If " + f.name + " was " + std::to_string(alternative)] = 
                    alternative;
            }
        }
        
        return counterfactuals;
    }
    
    std::vector<Feature> perturb_features(const std::vector<Feature>& features) {
        auto perturbed = features;
        
        for (auto& f : perturbed) {
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f;
            f.value += noise;
            f.value = std::max(0.0f, std::min(1.0f, f.value));
        }
        
        return perturbed;
    }
    
    float compute_distance(const std::vector<Feature>& f1, 
                          const std::vector<Feature>& f2) {
        float dist = 0.0f;
        
        for (size_t i = 0; i < std::min(f1.size(), f2.size()); i++) {
            float diff = f1[i].value - f2[i].value;
            dist += diff * diff;
        }
        
        return std::sqrt(dist);
    }
    
    std::vector<float> fit_linear_model(const std::vector<std::vector<Feature>>& X,
                                       const std::vector<std::string>& y,
                                       const std::vector<float>& weights,
                                       const std::string& target_class) {
        if (X.empty()) return {};
        
        int num_features = X[0].size();
        std::vector<float> coefficients(num_features, 0.0f);
        
        // Simplified linear regression (should use proper implementation)
        for (size_t i = 0; i < X.size(); i++) {
            float y_val = (y[i] == target_class) ? 1.0f : 0.0f;
            
            for (int j = 0; j < num_features; j++) {
                coefficients[j] += weights[i] * X[i][j].value * y_val;
            }
        }
        
        // Normalize
        float sum_weights = 0.0f;
        for (float w : weights) sum_weights += w;
        
        if (sum_weights > 0) {
            for (auto& c : coefficients) {
                c /= sum_weights;
            }
        }
        
        return coefficients;
    }
    
    float score_prediction(const std::string& prediction, const std::string& target) {
        return (prediction == target) ? 1.0f : 0.0f;
    }
};

} // namespace dnn::reasoning
