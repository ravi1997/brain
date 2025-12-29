#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace dnn::neural {

// Neural-Symbolic Integration - combining neural and symbolic reasoning
class NeuralSymbolicIntegration {
public:
    struct Symbol {
        std::string name;
        std::vector<float> embedding;  // Neural representation
        
        Symbol() {}
        Symbol(const std::string& n, const std::vector<float>& emb)
            : name(n), embedding(emb) {}
    };
    
    struct Rule {
        std::vector<std::string> premises;
        std::string conclusion;
        std::vector<float> learned_weights;
        
        Rule() {}
        Rule(const std::vector<std::string>& p, const std::string& c)
            : premises(p), conclusion(c) {}
    };
    
    NeuralSymbolicIntegration(int embedding_dim = 64)
        : embedding_dim_(embedding_dim) {}
    
    // Learn symbol embeddings from data
    void learn_embeddings(const std::vector<std::pair<std::string, std::vector<float>>>& examples) {
        for (const auto& [symbol, features] : examples) {
            // Simple embedding: average of feature vectors
            if (!symbol_embeddings_.count(symbol)) {
                symbol_embeddings_[symbol] = features;
            } else {
                // Update embedding
                for (size_t i = 0; i < features.size() && 
                     i < symbol_embeddings_[symbol].size(); i++) {
                    symbol_embeddings_[symbol][i] = 
                        0.9f * symbol_embeddings_[symbol][i] + 0.1f * features[i];
                }
            }
        }
    }
    
    // Add symbolic rule with learnable weights
    void add_rule(const Rule& rule) {
        rules_.push_back(rule);
        
        // Initialize weights
        if (rules_.back().learned_weights.empty()) {
            rules_.back().learned_weights.resize(rule.premises.size(), 1.0f);
        }
    }
    
    // Neural forward reasoning: apply rules with neural confidence
    std::vector<std::pair<std::string, float>> 
    forward_reason(const std::vector<std::string>& facts) {
        std::vector<std::pair<std::string, float>> conclusions;
        
        for (const auto& rule : rules_) {
            // Check if premises are satisfied
            float satisfaction = 0.0f;
            int count = 0;
            
            for (size_t i = 0; i < rule.premises.size(); i++) {
                const auto& premise = rule.premises[i];
                
                // Check if premise is in facts
                bool found = false;
                for (const auto& fact : facts) {
                    float similarity = compute_similarity(premise, fact);
                    if (similarity > 0.7f) {
                        found = true;
                        float weight = (i < rule.learned_weights.size()) ? 
                                      rule.learned_weights[i] : 1.0f;
                        satisfaction += similarity * weight;
                        count++;
                        break;
                    }
                }
            }
            
            // If enough premises satisfied, conclude
            if (count >= static_cast<int>(rule.premises.size()) * 0.8) {
                float confidence = count > 0 ? satisfaction / count : 0.0f;
                conclusions.emplace_back(rule.conclusion, confidence);
            }
        }
        
        return conclusions;
    }
    
    // Differentiable reasoning: soft logical operations
    float soft_and(const std::vector<float>& values) {
        float result = 1.0f;
        for (float v : values) {
            result *= v;
        }
        return result;
    }
    
    float soft_or(const std::vector<float>& values) {
        float result = 0.0f;
        for (float v : values) {
            result = result + v - result * v;  // Probabilistic OR
        }
        return result;
    }
    
    float soft_not(float value) {
        return 1.0f - value;
    }
    
    // Neural-symbolic query: combine embedding similarity with logic
    float query(const std::string& query_symbol,
               const std::vector<std::string>& context) {
        // Neural similarity
        float neural_score = 0.0f;
        
        for (const auto& ctx_symbol : context) {
            neural_score = std::max(neural_score, 
                                   compute_similarity(query_symbol, ctx_symbol));
        }
        
        // Symbolic reasoning
        auto conclusions = forward_reason(context);
        float symbolic_score = 0.0f;
        
        for (const auto& [conclusion, conf] : conclusions) {
            if (conclusion == query_symbol) {
                symbolic_score = std::max(symbolic_score, conf);
            }
        }
        
        // Combine scores
        return 0.5f * neural_score + 0.5f * symbolic_score;
    }
    
    // Learn rule weights from examples
    void learn_rule_weights(
        const std::vector<std::pair<std::vector<std::string>, std::string>>& examples,
        int epochs = 10,
        float learning_rate = 0.01f) {
        
        for (int epoch = 0; epoch < epochs; epoch++) {
            for (const auto& [facts, expected_conclusion] : examples) {
                // Forward pass
                auto conclusions = forward_reason(facts);
                
                // Find which rule produced the conclusion
                for (auto& rule : rules_) {
                    if (rule.conclusion == expected_conclusion) {
                        // Compute gradient and update weights
                        float predicted_conf = 0.0f;
                        
                        for (const auto& [concl, conf] : conclusions) {
                            if (concl == expected_conclusion) {
                                predicted_conf = conf;
                                break;
                            }
                        }
                        
                        float error = 1.0f - predicted_conf;  // Want confidence = 1
                        
                        // Update rule weights
                        for (auto& weight : rule.learned_weights) {
                            weight += learning_rate * error;
                            weight = std::max(0.0f, std::min(2.0f, weight));
                        }
                    }
                }
            }
        }
    }
    
private:
    int embedding_dim_;
    std::unordered_map<std::string, std::vector<float>> symbol_embeddings_;
    std::vector<Rule> rules_;
    
    float compute_similarity(const std::string& sym1, const std::string& sym2) {
        // Exact match
        if (sym1 == sym2) {
            return 1.0f;
        }
        
        // Neural embeddings similarity
        if (symbol_embeddings_.count(sym1) && symbol_embeddings_.count(sym2)) {
            return cosine_similarity(symbol_embeddings_[sym1],
                                    symbol_embeddings_[sym2]);
        }
        
        // String similarity fallback
        return string_similarity(sym1, sym2);
    }
    
    float cosine_similarity(const std::vector<float>& v1, const std::vector<float>& v2) {
        float dot = 0.0f, norm1 = 0.0f, norm2 = 0.0f;
        
        for (size_t i = 0; i < std::min(v1.size(), v2.size()); i++) {
            dot += v1[i] * v2[i];
            norm1 += v1[i] * v1[i];
            norm2 += v2[i] * v2[i];
        }
        
        if (norm1 == 0 || norm2 == 0) return 0.0f;
        
        return dot / (std::sqrt(norm1) * std::sqrt(norm2));
    }
    
    float string_similarity(const std::string& s1, const std::string& s2) {
        // Simple edit distance-based similarity
        int max_len = std::max(s1.length(), s2.length());
        if (max_len == 0) return 1.0f;
        
        int matches = 0;
        for (size_t i = 0; i < std::min(s1.length(), s2.length()); i++) {
            if (s1[i] == s2[i]) matches++;
        }
        
        return static_cast<float>(matches) / max_len;
    }
};

} // namespace dnn::neural
