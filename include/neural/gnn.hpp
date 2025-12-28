#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace dnn::neural {

// Graph Neural Network for relational reasoning (complete implementation)
class GraphNeuralNetwork {
public:
    using Graph = std::unordered_map<int, std::vector<int>>;
    
    enum class AggregationType {
        MEAN,      // Mean aggregation
        SUM,       // Sum aggregation
        MAX        // Max aggregation
    };
    
    GraphNeuralNetwork(int hidden_dim = 128, int num_layers = 2, 
                      AggregationType agg = AggregationType::MEAN)
        : hidden_dim_(hidden_dim), num_layers_(num_layers), aggregation_(agg) {
        
        // Initialize simple weight matrices
        for (int layer = 0; layer < num_layers; layer++) {
            std::vector<float> weights(hidden_dim * hidden_dim);
            for (auto& w : weights) {
                w = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
            }
            layer_weights_.push_back(weights);
        }
    }
    
    // Forward pass through all GNN layers
    std::unordered_map<int, std::vector<float>> forward(
        const Graph& graph,
        const std::unordered_map<int, std::vector<float>>& node_features) {
        
        auto current_features = node_features;
        
        // Apply each GNN layer
        for (int layer = 0; layer < num_layers_; layer++) {
            current_features = apply_layer(graph, current_features, layer);
        }
        
        return current_features;
    }
    
    // Apply single GNN layer
    std::unordered_map<int, std::vector<float>> apply_layer(
        const Graph& graph,
        const std::unordered_map<int, std::vector<float>>& node_features,
        int layer_idx) {
        
        std::unordered_map<int, std::vector<float>> new_features;
        
        for (const auto& [node_id, features] : node_features) {
            // Aggregate neighbor features
            auto aggregated = aggregate_neighbors(graph, node_id, node_features);
            
            // Combine with self features
            auto combined = combine_features(features, aggregated);
            
            // Apply transformation
            auto transformed = apply_transformation(combined, layer_idx);
            
            // Apply activation (ReLU)
            for (auto& val : transformed) {
                val = std::max(0.0f, val);
            }
            
            new_features[node_id] = transformed;
        }
        
        return new_features;
    }
    
    // Message passing: aggregate neighbor features
    std::vector<float> aggregate_neighbors(
        const Graph& graph,
        int node_id,
        const std::unordered_map<int, std::vector<float>>& node_features) const {
        
        auto it = node_features.find(node_id);
        if (it == node_features.end()) {
            return std::vector<float>(hidden_dim_, 0.0f);
        }
        
        auto neighbors_it = graph.find(node_id);
        if (neighbors_it == graph.end() || neighbors_it->second.empty()) {
            // No neighbors, return zeros
            return std::vector<float>(hidden_dim_, 0.0f);
        }
        
        std::vector<std::vector<float>> neighbor_features;
        for (int neighbor_id : neighbors_it->second) {
            auto neighbor_it = node_features.find(neighbor_id);
            if (neighbor_it != node_features.end()) {
                neighbor_features.push_back(neighbor_it->second);
            }
        }
        
        if (neighbor_features.empty()) {
            return std::vector<float>(hidden_dim_, 0.0f);
        }
        
        // Aggregate based on type
        std::vector<float> aggregated(hidden_dim_, 0.0f);
        
        switch (aggregation_) {
            case AggregationType::MEAN: {
                for (const auto& neighbor_feat : neighbor_features) {
                    for (size_t i = 0; i < aggregated.size() && i < neighbor_feat.size(); i++) {
                        aggregated[i] += neighbor_feat[i];
                    }
                }
                for (auto& val : aggregated) {
                    val /= neighbor_features.size();
                }
                break;
            }
            
            case AggregationType::SUM: {
                for (const auto& neighbor_feat : neighbor_features) {
                    for (size_t i = 0; i < aggregated.size() && i < neighbor_feat.size(); i++) {
                        aggregated[i] += neighbor_feat[i];
                    }
                }
                break;
            }
            
            case AggregationType::MAX: {
                aggregated = neighbor_features[0];
                for (size_t i = 1; i < neighbor_features.size(); i++) {
                    for (size_t j = 0; j < aggregated.size() && j < neighbor_features[i].size(); j++) {
                        aggregated[j] = std::max(aggregated[j], neighbor_features[i][j]);
                    }
                }
                break;
            }
        }
        
        return aggregated;
    }
    
    // Combine self and neighbor features
    std::vector<float> combine_features(const std::vector<float>& self_feat,
                                       const std::vector<float>& neighbor_feat) const {
        std::vector<float> combined(hidden_dim_, 0.0f);
        
        for (size_t i = 0; i < combined.size(); i++) {
            float self_val = i < self_feat.size() ? self_feat[i] : 0.0f;
            float neighbor_val = i < neighbor_feat.size() ? neighbor_feat[i] : 0.0f;
            combined[i] = self_val + neighbor_val;  // Simple sum
        }
        
        return combined;
    }
    
    // Apply learned transformation
    std::vector<float> apply_transformation(const std::vector<float>& features,
                                           int layer_idx) const {
        if (layer_idx >= static_cast<int>(layer_weights_.size())) {
            return features;
        }
        
        const auto& weights = layer_weights_[layer_idx];
        std::vector<float> output(hidden_dim_, 0.0f);
        
        // Matrix multiplication
        for (int i = 0; i < hidden_dim_; i++) {
            for (int j = 0; j < hidden_dim_ && j < static_cast<int>(features.size()); j++) {
                output[i] += weights[i * hidden_dim_ + j] * features[j];
            }
        }
        
        return output;
    }
    
    // Get node embedding after forward pass
    std::vector<float> get_node_embedding(
        const Graph& graph,
        int node_id,
        const std::unordered_map<int, std::vector<float>>& node_features) {
        
        auto output = forward(graph, node_features);
        auto it = output.find(node_id);
        return it != output.end() ? it->second : std::vector<float>(hidden_dim_, 0.0f);
    }
    
    // Graph-level readout (aggregate all node features)
    std::vector<float> graph_readout(
        const std::unordered_map<int, std::vector<float>>& node_embeddings) const {
        
        std::vector<float> graph_embedding(hidden_dim_, 0.0f);
        
        if (node_embeddings.empty()) {
            return graph_embedding;
        }
        
        // Mean pooling
        for (const auto& [node_id, embedding] : node_embeddings) {
            for (size_t i = 0; i < graph_embedding.size() && i < embedding.size(); i++) {
                graph_embedding[i] += embedding[i];
            }
        }
        
        for (auto& val : graph_embedding) {
            val /= node_embeddings.size();
        }
        
        return graph_embedding;
    }
    
private:
    int hidden_dim_;
    int num_layers_;
    AggregationType aggregation_;
    std::vector<std::vector<float>> layer_weights_;
};

} // namespace dnn::neural
