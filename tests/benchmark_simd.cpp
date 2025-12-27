#include <iostream>
#include <chrono>
#include <vector>
#include "simd_utils.hpp"
#include <random>

int main() {
    const size_t N = 1000000;
    std::vector<double> a(N), b(N);
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist(0, 1);
    for(size_t i=0; i<N; ++i) {
        a[i] = dist(gen);
        b[i] = dist(gen);
    }
    
    // Benchmark SIMD Dot Product
    auto start = std::chrono::high_resolution_clock::now();
    double sum = 0;
    for(int i=0; i<1000; ++i) {
        sum += dnn::simd::dot_product(a.data(), b.data(), N);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "SIMD Dot Product (1M elements x 1000 iterations): " << elapsed.count() << "s, result: " << sum << std::endl;
    
    // Compare with scalar
    start = std::chrono::high_resolution_clock::now();
    double scalar_sum = 0;
    for(int i=0; i<1000; ++i) {
        for(size_t j=0; j<N; ++j) {
            scalar_sum += a[j] * b[j];
        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Scalar Dot Product (1M elements x 1000 iterations): " << elapsed.count() << "s, result: " << scalar_sum << std::endl;
    
    return 0;
}
