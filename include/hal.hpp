#pragma once
#include <vector>
#include <cmath>

namespace dnn {

class HardwareAccelerator {
public:
    virtual ~HardwareAccelerator() = default;
    
    // Core Matrix Op: C = A * B
    virtual void matrix_multiply(const std::vector<std::vector<double>>& A, 
                                 const std::vector<std::vector<double>>& B, 
                                 std::vector<std::vector<double>>& C) = 0;
                                 
    virtual void dot_product_batch(const std::vector<double>& vec, 
                                   const std::vector<std::vector<double>>& batch, 
                                   std::vector<double>& results) = 0;
};

class CpuAccelerator : public HardwareAccelerator {
public:
    void matrix_multiply(const std::vector<std::vector<double>>& A, 
                         const std::vector<std::vector<double>>& B, 
                         std::vector<std::vector<double>>& C) override {
        // Standard Naive Implementation (for now)
        // In real AVX2 impl, this would use intrinsics
        size_t n = A.size();
        size_t m = B[0].size();
        size_t k = B.size();
        
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                C[i][j] = 0;
                for (size_t x = 0; x < k; ++x) {
                    C[i][j] += A[i][x] * B[x][j];
                }
            }
        }
    }

    void dot_product_batch(const std::vector<double>& vec, 
                           const std::vector<std::vector<double>>& batch, 
                           std::vector<double>& results) override {
         for (size_t i = 0; i < batch.size(); ++i) {
             double sum = 0;
             for (size_t j = 0; j < vec.size(); ++j) {
                 sum += vec[j] * batch[i][j];
             }
             results[i] = sum;
         }
    }
};

} // namespace dnn
