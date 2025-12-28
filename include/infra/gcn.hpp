#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace dnn::infra {

// Graph Convolutional Network for graph-structured data
class GraphConvolutionalNetwork {
public:
    using AdjacencyList = std::unordered_map<int, std::vector<int>>;
    
    GraphConvolutionalNetwork(int input_dim = 64, int output_dim = 32)
        : input_dim_(input_dim), output_dim_(output_dim) {
        // Initialize weight matrix (simplified)
        weights_.resize(input_dim * output_dim);
        for (auto& w : weights_) {
            w = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
        }
    }
    
    // Forward pass through one GCN layer
    std::unordered_map<int, std::vector<float>> forward(
        const AdjacencyList& graph,
        const std::unordered_map<int, std::vector<float>>& node_features) {
        
        std::unordered_map<int, std::vector<float>> output_features;
        
        for (const auto& [node_id, features] : node_features) {
            // Aggregate neighbor features
            std::vector<float> aggregated = aggregate_neighbors(graph, node_id, node_features);
            
            // Apply linear transformation
            std::vector<float> transformed = linear_transform(aggregated);
            
            // Apply ReLU activation
            for (auto& val : transformed) {
                val = std::max(0.0f, val);
            }
            
            output_features[node_id] = transformed;
        }
        
        return output_features;
    }
    
    // Message passing (aggregate neighbor features)
    std::vector<float> aggregate_neighbors(
        const AdjacencyList& graph,
        int node_id,
        const std::unordered_map<int, std::vector<float>>& node_features) const {
        
        auto it = node_features.find(node_id);
        if (it == node_features.end()) {
            return std::vector<float>(input_dim_, 0.0f);
        }
        
        std::vector<float> aggregated = it->second;
        int degree = 1;
        
        // Add neighbor features
        auto neighbors_it = graph.find(node_id);
        if (neighbors_it != graph.end()) {
            for (int neighbor_id : neighbors_it->second) {
                auto neighbor_it = node_features.find(neighbor_id);
                if (neighbor_it != node_features.end()) {
                    const auto& neighbor_features = neighbor_it->second;
                    for (size_t i = 0; i < aggregated.size() && i < neighbor_features.size(); i++) {
                        aggregated[i] += neighbor_features[i];
                    }
                    degree++;
                }
            }
        }
        
        // Normalize by degree (mean aggregation)
        for (auto& val : aggregated) {
            val /= degree;
        }
        
        return aggregated;
    }
    
    // Apply learned transformation
    std::vector<float> linear_transform(const std::vector<float>& input) const {
        std::vector<float> output(output_dim_, 0.0f);
        
        for (int i = 0; i < output_dim_; i++) {
            for (int j = 0; j < input_dim_ && j < static_cast<int>(input.size()); j++) {
                output[i] += weights_[i * input_dim_ + j] * input[j];
            }
        }
        
        return output;
    }
    
    // Get node embedding after forward pass
    std::vector<float> get_node_embedding(
        const AdjacencyList& graph,
        int node_id,
        const std::unordered_map<int, std::vector<float>>& node_features) {
        
        auto output = forward(graph, node_features);
        auto it = output.find(node_id);
        return it != output.end() ? it->second : std::vector<float>(output_dim_, 0.0f);
    }
    
private:
    int input_dim_;
    int output_dim_;
    std::vector<float> weights_;
};

} // namespace dnn::infra
