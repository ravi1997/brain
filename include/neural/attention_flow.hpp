#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <algorithm>

namespace dnn::neural {

// Attention Flow Visualization for neural network interpretability
class AttentionFlowVisualization {
public:
    struct AttentionHead {
        int head_id;
        std::vector<std::vector<float>> attention_weights;  // [query_len x key_len]
        int num_queries;
        int num_keys;
        
        AttentionHead() : head_id(0), num_queries(0), num_keys(0) {}
    };
    
    struct AttentionLayer {
        int layer_id;
        std::vector<AttentionHead> heads;
        
        AttentionLayer() : layer_id(0) {}
    };
    
    struct FlowPattern {
        std::string pattern_type;  // "local", "global", "syntactic", "semantic"
        float strength;
        std::vector<std::pair<int, int>> connections;  // (from, to) pairs
        
        FlowPattern() : strength(0.0f) {}
    };
    
    AttentionFlowVisualization() {}
    
    // Add attention weights from a layer
    void add_layer(int layer_id, const std::vector<AttentionHead>& heads) {
        AttentionLayer layer;
        layer.layer_id = layer_id;
        layer.heads = heads;
        layers_.push_back(layer);
    }
    
    // Compute attention flow across layers
    std::vector<std::vector<float>> compute_attention_flow() {
        if (layers_.empty()) {
            return {};
        }
        
        int seq_len = layers_[0].heads[0].num_queries;
        std::vector<std::vector<float>> flow(seq_len, std::vector<float>(seq_len, 0.0f));
        
        // Aggregate attention across all layers and heads
        for (const auto& layer : layers_) {
            for (const auto& head : layer.heads) {
                for (int i = 0; i < head.num_queries && i < seq_len; i++) {
                    for (int j = 0; j < head.num_keys && j < seq_len; j++) {
                        if (i < static_cast<int>(head.attention_weights.size()) &&
                            j < static_cast<int>(head.attention_weights[i].size())) {
                            flow[i][j] += head.attention_weights[i][j];
                        }
                    }
                }
            }
        }
        
        // Normalize
        int total_heads = 0;
        for (const auto& layer : layers_) {
            total_heads += layer.heads.size();
        }
        
        if (total_heads > 0) {
            for (auto& row : flow) {
                for (auto& val : row) {
                    val /= total_heads;
                }
            }
        }
        
        return flow;
    }
    
    // Detect attention patterns
    std::vector<FlowPattern> detect_patterns(float threshold = 0.1f) {
        std::vector<FlowPattern> patterns;
        auto flow = compute_attention_flow();
        
        if (flow.empty()) return patterns;
        
        int seq_len = flow.size();
        
        // Pattern 1: Local attention (diagonal)
        FlowPattern local;
        local.pattern_type = "local";
        local.strength = 0.0f;
        
        for (int i = 0; i < seq_len; i++) {
            for (int j = std::max(0, i-2); j <= std::min(seq_len-1, i+2); j++) {
                if (flow[i][j] > threshold) {
                    local.strength += flow[i][j];
                    local.connections.emplace_back(i, j);
                }
            }
        }
        local.strength /= (seq_len * 5);  // Normalize
        
        if (!local.connections.empty()) {
            patterns.push_back(local);
        }
        
        // Pattern 2: Global attention (attends to specific tokens)
        FlowPattern global;
        global.pattern_type = "global";
        global.strength = 0.0f;
        
        // Find tokens that receive a lot of attention
        std::vector<float> attention_received(seq_len, 0.0f);
        for (int i = 0; i < seq_len; i++) {
            for (int j = 0; j < seq_len; j++) {
                attention_received[j] += flow[i][j];
            }
        }
        
        float max_attention = *std::max_element(attention_received.begin(), 
                                               attention_received.end());
        
        for (int j = 0; j < seq_len; j++) {
            if (attention_received[j] > max_attention * 0.7f) {
                global.strength += attention_received[j];
                for (int i = 0; i < seq_len; i++) {
                    if (flow[i][j] > threshold) {
                        global.connections.emplace_back(i, j);
                    }
                }
            }
        }
        global.strength /= seq_len;
        
        if (!global.connections.empty()) {
            patterns.push_back(global);
        }
        
        return patterns;
    }
    
    // Generate heatmap for visualization
    std::vector<std::vector<float>> generate_heatmap(int layer_id = -1) {
        if (layer_id < 0) {
            return compute_attention_flow();
        }
        
        // Specific layer
        if (layer_id >= static_cast<int>(layers_.size())) {
            return {};
        }
        
        const auto& layer = layers_[layer_id];
        if (layer.heads.empty()) {
            return {};
        }
        
        int seq_len = layer.heads[0].num_queries;
        std::vector<std::vector<float>> heatmap(seq_len, std::vector<float>(seq_len, 0.0f));
        
        // Average across heads
        for (const auto& head : layer.heads) {
            for (int i = 0; i < head.num_queries && i < seq_len; i++) {
                for (int j = 0; j < head.num_keys && j < seq_len; j++) {
                    if (i < static_cast<int>(head.attention_weights.size()) &&
                        j < static_cast<int>(head.attention_weights[i].size())) {
                        heatmap[i][j] += head.attention_weights[i][j];
                    }
                }
            }
        }
        
        // Normalize
        for (auto& row : heatmap) {
            for (auto& val : row) {
                val /= layer.heads.size();
            }
        }
        
        return heatmap;
    }
    
    // Get attention entropy (measure of attention dispersion)
    std::vector<float> compute_attention_entropy() {
        std::vector<float> entropies;
        
        for (const auto& layer : layers_) {
            float layer_entropy = 0.0f;
            int count = 0;
            
            for (const auto& head : layer.heads) {
                for (const auto& row : head.attention_weights) {
                    float entropy = 0.0f;
                    for (float prob : row) {
                        if (prob > 0) {
                            entropy -= prob * std::log(prob);
                        }
                    }
                    layer_entropy += entropy;
                    count++;
                }
            }
            
            entropies.push_back(count > 0 ? layer_entropy / count : 0.0f);
        }
        
        return entropies;
    }
    
    // Identify most important tokens based on attention
    std::vector<int> get_important_tokens(int top_k = 5) {
        auto flow = compute_attention_flow();
        if (flow.empty()) return {};
        
        int seq_len = flow.size();
        std::vector<std::pair<float, int>> importance;
        
        // Sum attention received by each token
        for (int j = 0; j < seq_len; j++) {
            float total_attention = 0.0f;
            for (int i = 0; i < seq_len; i++) {
                total_attention += flow[i][j];
            }
            importance.emplace_back(total_attention, j);
        }
        
        // Sort and return top k
        std::sort(importance.begin(), importance.end(),
                 [](const auto& a, const auto& b) { return a.first > b.first; });
        
        std::vector<int> top_tokens;
        for (int i = 0; i < top_k && i < static_cast<int>(importance.size()); i++) {
            top_tokens.push_back(importance[i].second);
        }
        
        return top_tokens;
    }
    
private:
    std::vector<AttentionLayer> layers_;
};

} // namespace dnn::neural
