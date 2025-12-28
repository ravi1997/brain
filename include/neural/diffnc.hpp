#pragma once
#include <vector>
#include <cmath>

namespace dnn::neural {

// Differentiable Neural Computer with external memory
class DifferentiableNeuralComputer {
public:
    struct MemoryState {
        std::vector<std::vector<float>> memory;  // N x W memory matrix
        std::vector<float> read_weights;          // Read attention
        std::vector<float> write_weights;         // Write attention
        std::vector<float> usage;                 // Memory usage vector
        
        MemoryState(int n, int w) {
            memory.resize(n, std::vector<float>(w, 0.0f));
            read_weights.resize(n, 0.0f);
            write_weights.resize(n, 0.0f);
            usage.resize(n, 0.0f);
        }
    };
    
    DifferentiableNeuralComputer(int memory_size = 128, int memory_width = 20)
        : memory_size_(memory_size), memory_width_(memory_width),
          state_(memory_size, memory_width) {}
    
    // Content-based addressing
    std::vector<float> content_addressing(const std::vector<float>& key, float beta) const {
        std::vector<float> similarities(memory_size_);
        
        for (int i = 0; i < memory_size_; i++) {
            similarities[i] = cosine_similarity(key, state_.memory[i]);
        }
        
        // Apply temperature and softmax
        return softmax(similarities, beta);
    }
    
    // Read from memory
    std::vector<float> read(const std::vector<float>& read_key, float read_strength = 1.0f) {
        // Content-based read
        auto weights = content_addressing(read_key, read_strength);
        state_.read_weights = weights;
        
        // Weighted sum of memory
        std::vector<float> read_vector(memory_width_, 0.0f);
        for (int i = 0; i < memory_size_; i++) {
            for (int j = 0; j < memory_width_; j++) {
                read_vector[j] += weights[i] * state_.memory[i][j];
            }
        }
        
        return read_vector;
    }
    
    // Write to memory
    void write(const std::vector<float>& write_key, const std::vector<float>& write_vector,
              const std::vector<float>& erase_vector, float write_strength = 1.0f) {
        
        // Content-based write addressing
        auto weights = content_addressing(write_key, write_strength);
        state_.write_weights = weights;
        
        // Erase and write
        for (int i = 0; i < memory_size_; i++) {
            for (int j = 0; j < memory_width_; j++) {
                // Erase
                state_.memory[i][j] *= (1.0f - weights[i] * erase_vector[j]);
                
                // Add
                if (j < static_cast<int>(write_vector.size())) {
                    state_.memory[i][j] += weights[i] * write_vector[j];
                }
            }
            
            // Update usage
            state_.usage[i] = (1.0f - weights[i]) * state_.usage[i] + weights[i];
        }
    }
    
    // Dynamic memory allocation (find least used location)
    int allocate_memory() {
        int min_idx = 0;
        float min_usage = state_.usage[0];
        
        for (int i = 1; i < memory_size_; i++) {
            if (state_.usage[i] < min_usage) {
                min_usage = state_.usage[i];
                min_idx = i;
            }
        }
        
        return min_idx;
    }
    
    // Forward pass (simplified)
    std::vector<float> forward(const std::vector<float>& input) {
        if (input.size() < memory_width_) {
            return std::vector<float>(memory_width_, 0.0f);
        }
        
        // Use first part as read key, second as write
        std::vector<float> read_key(input.begin(), input.begin() + memory_width_);
        std::vector<float> write_key(input.begin(), input.begin() + memory_width_);
        std::vector<float> write_vec(input.begin(), input.begin() + memory_width_);
        std::vector<float> erase_vec(memory_width_, 0.1f);  // Small erase
        
        // Read from memory
        auto read_result = read(read_key);
        
        // Write to memory
        write(write_key, write_vec, erase_vec);
        
        return read_result;
    }
    
    // Reset memory
    void reset() {
        state_ = MemoryState(memory_size_, memory_width_);
    }
    
    // Get memory utilization
    float get_memory_utilization() const {
        float total_usage = 0.0f;
        for (float u : state_.usage) {
            total_usage += u;
        }
        return total_usage / memory_size_;
    }
    
private:
    int memory_size_;
    int memory_width_;
    MemoryState state_;
    
    float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) const {
        float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
        
        for (size_t i = 0; i < std::min(a.size(), b.size()); i++) {
            dot += a[i] * b[i];
            norm_a += a[i] * a[i];
        }
        
        for (float val : b) {
            norm_b += val * val;
        }
        
        float denom = std::sqrt(norm_a) * std::sqrt(norm_b);
        return denom > 0 ? dot / denom : 0.0f;
    }
    
    std::vector<float> softmax(const std::vector<float>& input, float temperature = 1.0f) const {
        if (input.empty()) return {};
        
        std::vector<float> scaled(input.size());
        float max_val = *std::max_element(input.begin(), input.end());
        
        float sum = 0.0f;
        for (size_t i = 0; i < input.size(); i++) {
            scaled[i] = std::exp((input[i] - max_val) * temperature);
            sum += scaled[i];
        }
        
        for (auto& val : scaled) {
            val /= sum;
        }
        
        return scaled;
    }
};

} // namespace dnn::neural
