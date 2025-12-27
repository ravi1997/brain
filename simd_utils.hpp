#pragma once
#include <vector>
#include <immintrin.h>

namespace dnn {
namespace simd {

    // Optimized dot product for double precision using AVX2
    inline double dot_product(const double* a, const double* b, size_t n) {
        __m256d sum = _mm256_setzero_pd();
        size_t i = 0;
        
        // Process in blocks of 4
        for (; i + 4 <= n; i += 4) {
            __m256d va = _mm256_loadu_pd(a + i);
            __m256d vb = _mm256_loadu_pd(b + i);
            sum = _mm256_fmadd_pd(va, vb, sum);
        }
        
        // Sum the contents of the 256-bit register
        alignas(32) double res[4];
        _mm256_store_pd(res, sum);
        double total = res[0] + res[1] + res[2] + res[3];
        
        // Handle remaining elements
        for (; i < n; ++i) {
            total += a[i] * b[i];
        }
        
        return total;
    }

    // Optimized vector addition: dest += src * scale
    inline void add_scaled(double* dest, const double* src, double scale, size_t n) {
        __m256d vscale = _mm256_set1_pd(scale);
        size_t i = 0;
        
        for (; i + 4 <= n; i += 4) {
            __m256d vd = _mm256_loadu_pd(dest + i);
            __m256d vs = _mm256_loadu_pd(src + i);
            vd = _mm256_fmadd_pd(vs, vscale, vd);
            _mm256_storeu_pd(dest + i, vd);
        }
        
        for (; i < n; ++i) {
            dest[i] += src[i] * scale;
        }
    }

} // namespace simd
} // namespace dnn
