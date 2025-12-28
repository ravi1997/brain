#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace dnn::infra {

// Quantized Inference for low-precision neural network execution
class QuantizedInference {
public:
    enum class QuantizationType {
        INT8,       // 8-bit integer
        INT4,       // 4-bit integer (packed)
        FLOAT16     // 16-bit floating point
    };
    
    QuantizedInference(QuantizationType type = QuantizationType::INT8)
        : quant_type_(type) {}
    
    // Quantize float tensor to int8
    std::vector<int8_t> quantize_int8(const std::vector<float>& input, float& scale, int8_t& zero_point) {
        if (input.empty()) return {};
        
        // Find min and max
        float min_val = *std::min_element(input.begin(), input.end());
        float max_val = *std::max_element(input.begin(), input.end());
        
        // Calculate scale and zero point
        scale = (max_val - min_val) / 255.0f;
        if (scale == 0) scale = 1.0f;
        zero_point = static_cast<int8_t>(-128 - std::round(min_val / scale));
        
        // Quantize
        std::vector<int8_t> quantized(input.size());
        for (size_t i = 0; i < input.size(); i++) {
            float scaled = input[i] / scale + zero_point;
            quantized[i] = static_cast<int8_t>(std::max(-128.0f, std::min(127.0f, std::round(scaled))));
        }
        
        return quantized;
    }
    
    // Dequantize int8 to float
    std::vector<float> dequantize_int8(const std::vector<int8_t>& input, float scale, int8_t zero_point) {
        std::vector<float> output(input.size());
        for (size_t i = 0; i < input.size(); i++) {
            output[i] = scale * (input[i] - zero_point);
        }
        return output;
    }
    
    // Quantize to symmetric int8 (zero_point = 0)
    std::vector<int8_t> quantize_symmetric(const std::vector<float>& input, float& scale) {
        if (input.empty()) return {};
        
        float abs_max = 0.0f;
        for (float val : input) {
            abs_max = std::max(abs_max, std::abs(val));
        }
        
        scale = abs_max / 127.0f;
        if (scale == 0) scale = 1.0f;
        
        std::vector<int8_t> quantized(input.size());
        for (size_t i = 0; i < input.size(); i++) {
            float scaled = input[i] / scale;
            quantized[i] = static_cast<int8_t>(std::max(-128.0f, std::min(127.0f, std::round(scaled))));
        }
        
        return quantized;
    }
    
    // Quantized matrix multiplication (int8)
    std::vector<int32_t> matmul_int8(
        const std::vector<int8_t>& A, int m, int k,
        const std::vector<int8_t>& B, int n,
        float scale_a, float scale_b, int8_t zero_a = 0, int8_t zero_b = 0) {
        
        std::vector<int32_t> C(m * n, 0);
        
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                int32_t sum = 0;
                for (int p = 0; p < k; p++) {
                    int32_t a_val = A[i * k + p] - zero_a;
                    int32_t b_val = B[p * n + j] - zero_b;
                    sum += a_val * b_val;
                }
                C[i * n + j] = sum;
            }
        }
        
        return C;
    }
    
    // Rescale quantized result to float
    std::vector<float> rescale_int32(const std::vector<int32_t>& input,
                                    float scale_a, float scale_b, float scale_out) {
        float scale = (scale_a * scale_b) / scale_out;
        
        std::vector<float> output(input.size());
        for (size_t i = 0; i < input.size(); i++) {
            output[i] = input[i] * scale;
        }
        
        return output;
    }
    
    // Per-channel quantization for weights
    struct PerChannelQuantization {
        std::vector<int8_t> weights;
        std::vector<float> scales;
        std::vector<int8_t> zero_points;
    };
    
    PerChannelQuantization quantize_per_channel(const std::vector<float>& weights,
                                                int num_channels, int channel_size) {
        PerChannelQuantization result;
        result.weights.resize(weights.size());
        result.scales.resize(num_channels);
        result.zero_points.resize(num_channels);
        
        for (int c = 0; c < num_channels; c++) {
            int offset = c * channel_size;
            
            // Find min/max for this channel
            float min_val = weights[offset];
            float max_val = weights[offset];
            for (int i = 1; i < channel_size && offset + i < static_cast<int>(weights.size()); i++) {
                min_val = std::min(min_val, weights[offset + i]);
                max_val = std::max(max_val, weights[offset + i]);
            }
            
            // Calculate scale and zero point
            float scale = (max_val - min_val) / 255.0f;
            if (scale == 0) scale = 1.0f;
            int8_t zero_point = static_cast<int8_t>(-128 - std::round(min_val / scale));
            
            result.scales[c] = scale;
            result.zero_points[c] = zero_point;
            
            // Quantize channel
            for (int i = 0; i < channel_size && offset + i < static_cast<int>(weights.size()); i++) {
                float scaled = weights[offset + i] / scale + zero_point;
                result.weights[offset + i] = static_cast<int8_t>(
                    std::max(-128.0f, std::min(127.0f, std::round(scaled)))
                );
            }
        }
        
        return result;
    }
    
    // Calculate compression ratio
    float get_compression_ratio() const {
        switch (quant_type_) {
            case QuantizationType::INT8:
                return 32.0f / 8.0f;  // 4x compression from FP32
            case QuantizationType::INT4:
                return 32.0f / 4.0f;  // 8x compression
            case QuantizationType::FLOAT16:
                return 32.0f / 16.0f; // 2x compression
        }
        return 1.0f;
    }
    
private:
    QuantizationType quant_type_;
};

} // namespace dnn::infra
