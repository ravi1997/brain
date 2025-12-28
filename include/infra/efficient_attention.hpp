#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace dnn::infra {

// O(n) Linear Attention using feature maps
class EfficientAttention {
public:
    EfficientAttention(int dim = 64, int num_features = 128)
        : dim_(dim), num_features_(num_features) {}
    
    // Forward pass with O(n*d^2) complexity instead of O(n^2*d)
    std::vector<float> forward(
        const std::vector<std::vector<float>>& Q,  // n x d
        const std::vector<std::vector<float>>& K,  // n x d
        const std::vector<std::vector<float>>& V   // n x d
    ) {
        if (Q.empty() || K.empty() || V.empty()) {
            return {};
        }
        
        int n = Q.size();
        
        // Apply feature maps: φ(Q), φ(K)
        auto phi_Q = apply_feature_map(Q);  // n x f
        auto phi_K = apply_feature_map(K);  // n x f
        
        // Compute K^T * V  (f x d)
        auto KV = compute_KV(phi_K, V);
        
        // Compute Q * KV  (n x d)
        auto result = compute_QKV(phi_Q, KV);
        
        // Flatten to 1D for return
        std::vector<float> flattened;
        flattened.reserve(n * dim_);
        for (const auto& row : result) {
            flattened.insert(flattened.end(), row.begin(), row.end());
        }
        
        return flattened;
    }
    
    // Single-query attention (more common use case)
    std::vector<float> attend(
        const std::vector<float>& query,          // d
        const std::vector<std::vector<float>>& keys,    // n x d
        const std::vector<std::vector<float>>& values   // n x d
    ) {
        if (keys.empty() || values.empty()) {
            return std::vector<float>(dim_, 0.0f);
        }
        
        // Apply feature map to query
        auto phi_q = feature_map(query);
        
        // Apply feature map to keys
        std::vector<std::vector<float>> phi_K;
        phi_K.reserve(keys.size());
        for (const auto& k : keys) {
            phi_K.push_back(feature_map(k));
        }
        
        // Compute K^T * V
        auto KV = compute_KV(phi_K, values);
        
        // Compute φ(q) * KV
        std::vector<float> result(dim_, 0.0f);
        for (int j = 0; j < dim_; j++) {
            for (int f = 0; f < num_features_; f++) {
                result[j] += phi_q[f] * KV[f][j];
            }
        }
        
        // Normalize
        float sum = 0.0f;
        for (int f = 0; f < num_features_; f++) {
            for (const auto& phi_k : phi_K) {
                sum += phi_q[f] * phi_k[f];
            }
        }
        
        if (sum > 1e-6f) {
            for (auto& v : result) {
                v /= sum;
            }
        }
        
        return result;
    }
    
private:
    int dim_;
    int num_features_;
    
    // ELU-based feature map for positive features
    std::vector<float> feature_map(const std::vector<float>& x) const {
        std::vector<float> features(num_features_);
        
        // Use ELU activation: elu(x) = x if x > 0, else exp(x) - 1
        // We add 1 for positive features
        for (int i = 0; i < std::min(static_cast<int>(x.size()), num_features_); i++) {
            features[i] = x[i] >= 0 ? x[i] + 1.0f : std::exp(x[i]);
        }
        
        // Random Fourier Features for remaining dimensions
        for (int i = x.size(); i < num_features_; i++) {
            float sum = 0.0f;
            for (size_t j = 0; j < x.size(); j++) {
                // Simple hash-based random projection
                float w = std::sin(static_cast<float>(i * 13 + j * 17));
                sum += w * x[j];
            }
            features[i] = std::cos(sum);
        }
        
        return features;
    }
    
    // Apply feature map to all queries/keys
    std::vector<std::vector<float>> apply_feature_map(
        const std::vector<std::vector<float>>& X
    ) const {
        std::vector<std::vector<float>> phi_X;
        phi_X.reserve(X.size());
        
        for (const auto& x : X) {
            phi_X.push_back(feature_map(x));
        }
        
        return phi_X;
    }
    
    // Compute φ(K)^T * V  (f x d)
    std::vector<std::vector<float>> compute_KV(
        const std::vector<std::vector<float>>& phi_K,  // n x f
        const std::vector<std::vector<float>>& V        // n x d
    ) const {
        std::vector<std::vector<float>> KV(num_features_, std::vector<float>(dim_, 0.0f));
        
        int n = phi_K.size();
        for (int f = 0; f < num_features_; f++) {
            for (int d = 0; d < dim_; d++) {
                for (int i = 0; i < n; i++) {
                    KV[f][d] += phi_K[i][f] * V[i][d];
                }
            }
        }
        
        return KV;
    }
    
    // Compute φ(Q) * KV  (n x d)
    std::vector<std::vector<float>> compute_QKV(
        const std::vector<std::vector<float>>& phi_Q,  // n x f
        const std::vector<std::vector<float>>& KV       // f x d
    ) const {
        int n = phi_Q.size();
        std::vector<std::vector<float>> result(n, std::vector<float>(dim_, 0.0f));
        
        for (int i = 0; i < n; i++) {
            for (int d = 0; d < dim_; d++) {
                for (int f = 0; f < num_features_; f++) {
                    result[i][d] += phi_Q[i][f] * KV[f][d];
                }
            }
        }
        
        return result;
    }
};

} // namespace dnn::infra
