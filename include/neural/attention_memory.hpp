#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <chrono>

namespace dnn::neural {

// Attention-based memory retrieval with real functionality
class AttentionMemory {
public:
    struct MemoryEntry {
        std::vector<float> key;      // Query key for attention
        std::vector<float> value;    // Stored value
        float timestamp;              // When added
        float importance;             // Priority weight
        
        MemoryEntry(const std::vector<float>& k, const std::vector<float>& v, 
                   float imp = 1.0f)
            : key(k), value(v), importance(imp) {
            timestamp = static_cast<float>(
                std::chrono::steady_clock::now().time_since_epoch().count()
            );
        }
    };
    
    AttentionMemory(int memory_size = 256, int dim = 64) 
        : max_memory_size_(memory_size), embedding_dim_(dim) {}
    
    // Store a memory with key-value pair
    void store(const std::vector<float>& key, const std::vector<float>& value, 
              float importance = 1.0f) {
        if (key.size() != embedding_dim_ || value.size() != embedding_dim_) {
            return; // Dimension mismatch
        }
        
        memory_.emplace_back(key, value, importance);
        
        // Evict oldest if over capacity
        if (memory_.size() > max_memory_size_) {
            memory_.erase(memory_.begin());
        }
    }
    
    // Retrieve memories using attention mechanism
    std::vector<float> retrieve(const std::vector<float>& query, int top_k = 5) {
        if (memory_.empty() || query.size() != embedding_dim_) {
            return std::vector<float>(embedding_dim_, 0.0f);
        }
        
        // Compute attention scores for all memories
        std::vector<float> scores;
        scores.reserve(memory_.size());
        
        for (const auto& entry : memory_) {
            float score = dot_product(query, entry.key);
            score *= entry.importance;  // Weight by importance
            scores.push_back(score);
        }
        
        // Apply softmax to get attention weights
        auto weights = softmax(scores);
        
        // Get top-k indices
        std::vector<int> top_indices = get_top_k_indices(weights, top_k);
        
        // Weighted sum of values
        std::vector<float> result(embedding_dim_, 0.0f);
        for (int idx : top_indices) {
            float weight = weights[idx];
            for (size_t i = 0; i < embedding_dim_; i++) {
                result[i] += weight * memory_[idx].value[i];
            }
        }
        
        return result;
    }
    
    // Cosine similarity-based retrieval (alternative to dot product)
    std::vector<float> retrieve_cosine(const std::vector<float>& query, int top_k = 5) {
        if (memory_.empty() || query.size() != embedding_dim_) {
            return std::vector<float>(embedding_dim_, 0.0f);
        }
        
        std::vector<float> scores;
        scores.reserve(memory_.size());
        
        float query_norm = l2_norm(query);
        
        for (const auto& entry : memory_) {
            float similarity = cosine_similarity(query, entry.key, query_norm);
            similarity *= entry.importance;
            scores.push_back(similarity);
        }
        
        auto weights = softmax(scores);
        std::vector<int> top_indices = get_top_k_indices(weights, top_k);
        
        std::vector<float> result(embedding_dim_, 0.0f);
        for (int idx : top_indices) {
            float weight = weights[idx];
            for (size_t i = 0; i < embedding_dim_; i++) {
                result[i] += weight * memory_[idx].value[i];
            }
        }
        
        return result;
    }
    
    // Clear all memories
    void clear() {
        memory_.clear();
    }
    
    // Get number of stored memories
    size_t size() const {
        return memory_.size();
    }
    
private:
    int max_memory_size_;
    int embedding_dim_;
    std::vector<MemoryEntry> memory_;
    
    // Dot product between two vectors
    float dot_product(const std::vector<float>& a, const std::vector<float>& b) const {
        float sum = 0.0f;
        for (size_t i = 0; i < a.size(); i++) {
            sum += a[i] * b[i];
        }
        return sum;
    }
    
    // L2 norm of vector
    float l2_norm(const std::vector<float>& v) const {
        return std::sqrt(dot_product(v, v));
    }
    
    // Cosine similarity
    float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b,
                           float a_norm = -1.0f) const {
        if (a_norm < 0) a_norm = l2_norm(a);
        float b_norm = l2_norm(b);
        
        if (a_norm == 0 || b_norm == 0) return 0.0f;
        
        return dot_product(a, b) / (a_norm * b_norm);
    }
    
    // Softmax function
    std::vector<float> softmax(const std::vector<float>& scores) const {
        if (scores.empty()) return {};
        
        // Find max for numerical stability
        float max_score = *std::max_element(scores.begin(), scores.end());
        
        std::vector<float> exp_scores(scores.size());
        float sum = 0.0f;
        
        for (size_t i = 0; i < scores.size(); i++) {
            exp_scores[i] = std::exp(scores[i] - max_score);
            sum += exp_scores[i];
        }
        
        if (sum > 0) {
            for (auto& s : exp_scores) {
                s /= sum;
            }
        }
        
        return exp_scores;
    }
    
    // Get indices of top-k elements
    std::vector<int> get_top_k_indices(const std::vector<float>& values, int k) const {
        std::vector<std::pair<float, int>> indexed_values;
        indexed_values.reserve(values.size());
        
        for (size_t i = 0; i < values.size(); i++) {
            indexed_values.emplace_back(values[i], i);
        }
        
        // Partial sort to get top-k
        int actual_k = std::min(k, static_cast<int>(values.size()));
        std::partial_sort(
            indexed_values.begin(),
            indexed_values.begin() + actual_k,
            indexed_values.end(),
            [](const auto& a, const auto& b) { return a.first > b.first; }
        );
        
        std::vector<int> indices;
        indices.reserve(actual_k);
        for (int i = 0; i < actual_k; i++) {
            indices.push_back(indexed_values[i].second);
        }
        
        return indices;
    }
};

} // namespace dnn::neural
