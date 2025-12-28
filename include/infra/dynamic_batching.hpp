#pragma once
#include <vector>
#include <cstdint>

namespace dnn::infra {

// Simple dynamic batching for float vectors
class DynamicBatching {
public:
    DynamicBatching(int max_batch = 32) : max_batch_size_(max_batch) {}
    
    std::vector<std::vector<float>> batch(const std::vector<float>& input) {
        return {input};
    }
    
private:
    int max_batch_size_;
};

} // namespace dnn::infra
