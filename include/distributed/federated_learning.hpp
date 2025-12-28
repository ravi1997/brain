#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <numeric>
#include <cmath>
#include <random>
#include <algorithm>

namespace dnn::distributed {

// Federated Learning for privacy-preserving distributed training
class FederatedLearning {
public:
    using ModelWeights = std::vector<float>;
    
    enum class AggregationStrategy {
        FEDERATED_AVERAGING,  // FedAvg: weighted average by data size
        FEDERATED_MEDIAN,     // More robust to outliers
        WEIGHTED_AVERAGE      // Custom weights per client
    };
    
    struct ClientUpdate {
        std::string client_id;
        ModelWeights weights;
        int num_samples;
        float loss;
        
        ClientUpdate(const std::string& id, const ModelWeights& w, int n, float l = 0.0f)
            : client_id(id), weights(w), num_samples(n), loss(l) {}
    };
    
    FederatedLearning(int model_size, AggregationStrategy strategy = AggregationStrategy::FEDERATED_AVERAGING)
        : model_size_(model_size), strategy_(strategy), global_round_(0) {
        global_weights_.resize(model_size, 0.0f);
    }
    
    // Get current global model
    ModelWeights get_global_model() const {
        return global_weights_;
    }
    
    // Aggregate updates from multiple clients
    void aggregate(const std::vector<ClientUpdate>& client_updates) {
        if (client_updates.empty()) {
            return;
        }
        
        switch (strategy_) {
            case AggregationStrategy::FEDERATED_AVERAGING:
                federated_averaging(client_updates);
                break;
            case AggregationStrategy::FEDERATED_MEDIAN:
                federated_median(client_updates);
                break;
            case AggregationStrategy::WEIGHTED_AVERAGE:
                weighted_average(client_updates);
                break;
        }
        
        global_round_++;
    }
    
    // Single client update (convenience method)
    void add_client_update(const std::string& client_id, const ModelWeights& weights, 
                          int num_samples, float loss = 0.0f) {
        client_updates_buffer_.emplace_back(client_id, weights, num_samples, loss);
    }
    
    // Aggregate buffered updates and clear buffer
    void aggregate_buffered() {
        aggregate(client_updates_buffer_);
        client_updates_buffer_.clear();
    }
    
    // Get current round number
    int get_round() const {
        return global_round_;
    }
    
    // Set custom client weights
    void set_client_weight(const std::string& client_id, float weight) {
        client_weights_[client_id] = weight;
    }
    
    // Compute differential privacy noise (simplified)
    ModelWeights add_dp_noise(const ModelWeights& weights, float noise_scale) const {
        ModelWeights noisy_weights = weights;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> dist(0.0f, noise_scale);
        
        for (auto& w : noisy_weights) {
            w += dist(gen);
        }
        
        return noisy_weights;
    }
    
    // Compute model divergence (L2 distance from global model)
    float compute_divergence(const ModelWeights& client_weights) const {
        if (client_weights.size() != global_weights_.size()) {
            return std::numeric_limits<float>::max();
        }
        
        float sum = 0.0f;
        for (size_t i = 0; i < global_weights_.size(); i++) {
            float diff = client_weights[i] - global_weights_[i];
            sum += diff * diff;
        }
        
        return std::sqrt(sum);
    }
    
    // Get statistics
    struct Stats {
        int num_clients;
        int total_samples;
        float avg_loss;
        float model_norm;
    };
    
    Stats get_last_round_stats() const {
        return last_stats_;
    }
    
private:
    int model_size_;
    AggregationStrategy strategy_;
    ModelWeights global_weights_;
    int global_round_;
    
    std::vector<ClientUpdate> client_updates_buffer_;
    std::unordered_map<std::string, float> client_weights_;
    Stats last_stats_;
    
    // FedAvg: Weighted average by number of samples
    void federated_averaging(const std::vector<ClientUpdate>& updates) {
        int total_samples = 0;
        for (const auto& update : updates) {
            total_samples += update.num_samples;
        }
        
        if (total_samples == 0) {
            return;
        }
        
        // Reset global weights
        std::fill(global_weights_.begin(), global_weights_.end(), 0.0f);
        
        float total_loss = 0.0f;
        
        // Weighted sum
        for (const auto& update : updates) {
            float weight = static_cast<float>(update.num_samples) / total_samples;
            
            for (size_t i = 0; i < global_weights_.size() && i < update.weights.size(); i++) {
                global_weights_[i] += weight * update.weights[i];
            }
            
            total_loss += weight * update.loss;
        }
        
        // Update stats
        last_stats_.num_clients = updates.size();
        last_stats_.total_samples = total_samples;
        last_stats_.avg_loss = total_loss;
        last_stats_.model_norm = compute_model_norm(global_weights_);
    }
    
    // FedMedian: Component-wise median for robustness
    void federated_median(const std::vector<ClientUpdate>& updates) {
        std::vector<std::vector<float>> all_weights;
        
        for (const auto& update : updates) {
            all_weights.push_back(update.weights);
        }
        
        // Compute component-wise median
        for (size_t i = 0; i < global_weights_.size(); i++) {
            std::vector<float> component_values;
            component_values.reserve(all_weights.size());
            
            for (const auto& weights : all_weights) {
                if (i < weights.size()) {
                    component_values.push_back(weights[i]);
                }
            }
            
            if (!component_values.empty()) {
                std::sort(component_values.begin(), component_values.end());
                size_t mid = component_values.size() / 2;
                
                if (component_values.size() % 2 == 0) {
                    global_weights_[i] = (component_values[mid - 1] + component_values[mid]) / 2.0f;
                } else {
                    global_weights_[i] = component_values[mid];
                }
            }
        }
        
        // Update stats
        last_stats_.num_clients = updates.size();
        last_stats_.total_samples = 0;
        for (const auto& u : updates) {
            last_stats_.total_samples += u.num_samples;
        }
        last_stats_.avg_loss = 0.0f;
        for (const auto& u : updates) {
            last_stats_.avg_loss += u.loss;
        }
        last_stats_.avg_loss /= updates.size();
        last_stats_.model_norm = compute_model_norm(global_weights_);
    }
    
    // Weighted average with custom client weights
    void weighted_average(const std::vector<ClientUpdate>& updates) {
        float total_weight = 0.0f;
        
        for (const auto& update : updates) {
            float weight = 1.0f;
            auto it = client_weights_.find(update.client_id);
            if (it != client_weights_.end()) {
                weight = it->second;
            }
            total_weight += weight;
        }
        
        if (total_weight == 0.0f) {
            return;
        }
        
        // Reset global weights
        std::fill(global_weights_.begin(), global_weights_.end(), 0.0f);
        
        // Weighted sum
        for (const auto& update : updates) {
            float weight = 1.0f;
            auto it = client_weights_.find(update.client_id);
            if (it != client_weights_.end()) {
                weight = it->second;
            }
            weight /= total_weight;
            
            for (size_t i = 0; i < global_weights_.size() && i < update.weights.size(); i++) {
                global_weights_[i] += weight * update.weights[i];
            }
        }
        
        // Update stats
        last_stats_.num_clients = updates.size();
        last_stats_.total_samples = 0;
        for (const auto& u : updates) {
            last_stats_.total_samples += u.num_samples;
        }
        last_stats_.avg_loss = 0.0f;
        for (const auto& u : updates) {
            last_stats_.avg_loss += u.loss;
        }
        last_stats_.avg_loss /= updates.size();
        last_stats_.model_norm = compute_model_norm(global_weights_);
    }
    
    float compute_model_norm(const ModelWeights& weights) const {
        float sum = 0.0f;
        for (float w : weights) {
            sum += w * w;
        }
        return std::sqrt(sum);
    }
};

} // namespace dnn::distributed
