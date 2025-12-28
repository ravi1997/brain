#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace dnn::infra {

// CUDA Kernel Optimization Utilities
class CUDAKernels {
public:
    // Matrix multiplication optimization parameters
    struct MatMulConfig {
        int block_size_x;
        int block_size_y;
        int tile_size;
        bool use_shared_memory;
        bool use_tensor_cores;
        
        MatMulConfig() : block_size_x(16), block_size_y(16), 
                        tile_size(16), use_shared_memory(true),
                        use_tensor_cores(false) {}
    };
    
    // Convolution parameters
    struct ConvConfig {
        int kernel_size;
        int stride;
        int padding;
        int groups;  // For grouped convolution
        
        ConvConfig() : kernel_size(3), stride(1), padding(1), groups(1) {}
    };
    
    CUDAKernels() {}
    
    // Calculate optimal block dimensions for 2D grid
    static std::pair<int, int> calculate_block_dim(int width, int height, 
                                                    int max_threads = 1024) {
        // Common block sizes
        std::vector<std::pair<int, int>> candidates = {
            {32, 32}, {16, 16}, {8, 8}, {32, 16}, {16, 32}
        };
        
        for (const auto& [bx, by] : candidates) {
            if (bx * by <= max_threads) {
                return {bx, by};
            }
        }
        
        return {16, 16};  // Default fallback
    }
    
    // Calculate grid dimensions
    static std::pair<int, int> calculate_grid_dim(int width, int height,
                                                   int block_x, int block_y) {
        int grid_x = (width + block_x - 1) / block_x;
        int grid_y = (height + block_y - 1) / block_y;
        return {grid_x, grid_y};
    }
    
    // Optimize matrix multiplication tile size
    static int optimize_tile_size(int m, int n, int k, int shared_mem_size = 49152) {
        // Shared memory per SM (typical: 48KB)
        std::vector<int> tile_sizes = {32, 16, 8, 4};
        
        for (int tile : tile_sizes) {
            // Two tiles in shared memory (A and B)
            int required_mem = 2 * tile * tile * sizeof(float);
            if (required_mem <= shared_mem_size) {
                return tile;
            }
        }
        
        return 8;  // Minimum fallback
    }
    
    // Calculate occupancy
    static float calculate_occupancy(int threads_per_block, int blocks_per_sm,
                                    int max_threads_per_sm = 2048) {
        int active_threads = threads_per_block * blocks_per_sm;
        return static_cast<float>(active_threads) / max_threads_per_sm;
    }
    
    // Optimize reduction operation
    static int optimize_reduction_block_size(int num_elements) {
        // Powers of 2 for efficient reduction
        std::vector<int> block_sizes = {1024, 512, 256, 128, 64, 32};
        
        for (int size : block_sizes) {
            if (num_elements >= size * 2) {
                return size;
            }
        }
        
        return 32;
    }
    
    // Calculate memory bandwidth utilization
    static float calculate_bandwidth_utilization(int bytes_transferred,
                                                 float time_ms,
                                                 float peak_bandwidth_gbps = 900.0f) {
        // Convert to GB/s
        float achieved_bandwidth = (bytes_transferred / 1e9f) / (time_ms / 1000.0f);
        return (achieved_bandwidth / peak_bandwidth_gbps) * 100.0f;
    }
    
    // Optimize convolution parameters
    static ConvConfig optimize_conv_params(int input_h, int input_w, 
                                          int output_h, int output_w) {
        ConvConfig config;
        
        // Infer kernel size from dimensions
        int h_diff = input_h - output_h;
        int w_diff = input_w - output_w;
        
        if (h_diff == 0 && w_diff == 0) {
            // Same padding
            config.kernel_size = 3;
            config.stride = 1;
            config.padding = 1;
        } else if (h_diff == 2 && w_diff == 2) {
            // Valid padding
            config.kernel_size = 3;
            config.stride = 1;
            config.padding = 0;
        } else {
            // Strided convolution
            config.kernel_size = 3;
            config.stride = 2;
            config.padding = 1;
        }
        
        return config;
    }
    
    // Calculate FLOPS for matrix multiplication
    static int64_t calculate_matmul_flops(int m, int n, int k) {
        // Each output element requires k multiplications and k-1 additions
        return static_cast<int64_t>(m) * n * (2 * k - 1);
    }
    
    // Calculate FLOPS for convolution
    static int64_t calculate_conv_flops(int batch, int out_channels, int out_h, int out_w,
                                        int in_channels, int kernel_h, int kernel_w) {
        return static_cast<int64_t>(batch) * out_channels * out_h * out_w * 
               in_channels * kernel_h * kernel_w * 2;  // MAC operations
    }
    
    // Coalescing check for memory access
    static bool is_coalesced_access(int thread_id, int stride, int element_size = 4) {
        // Check if memory accesses are coalesced (aligned)
        int offset = thread_id * stride * element_size;
        return (offset % 128) == 0;  // 128-byte cache line
    }
    
    // Bank conflict detection for shared memory
    static int count_bank_conflicts(const std::vector<int>& access_pattern,
                                    int num_banks = 32) {
        std::vector<int> bank_access_count(num_banks, 0);
        
        for (int addr : access_pattern) {
            int bank = (addr / 4) % num_banks;  // 4-byte words
            bank_access_count[bank]++;
        }
        
        int conflicts = 0;
        for (int count : bank_access_count) {
            if (count > 1) {
                conflicts += (count - 1);
            }
        }
        
        return conflicts;
    }
    
    // Warp divergence estimation
    static float estimate_warp_divergence(const std::vector<bool>& thread_conditions) {
        if (thread_conditions.empty()) return 0.0f;
        
        int warp_size = 32;
        int num_warps = (thread_conditions.size() + warp_size - 1) / warp_size;
        int divergent_warps = 0;
        
        for (int w = 0; w < num_warps; w++) {
            bool first_cond = thread_conditions[w * warp_size];
            for (int t = 1; t < warp_size && w * warp_size + t < static_cast<int>(thread_conditions.size()); t++) {
                if (thread_conditions[w * warp_size + t] != first_cond) {
                    divergent_warps++;
                    break;
                }
            }
        }
        
        return static_cast<float>(divergent_warps) / num_warps;
    }
    
    // Recommend optimization strategy
    struct OptimizationRecommendation {
        bool use_shared_memory;
        bool use_texture_memory;
        bool use_constant_memory;
        int recommended_block_size;
        std::string strategy;
    };
    
    static OptimizationRecommendation recommend_optimization(
        int64_t data_size, bool read_only, bool reused) {
        
        OptimizationRecommendation rec;
        rec.use_shared_memory = reused && (data_size < 48 * 1024);  // Fits in shared mem
        rec.use_texture_memory = read_only && !rec.use_shared_memory;
        rec.use_constant_memory = read_only && (data_size < 64 * 1024);  // 64KB constant mem
        rec.recommended_block_size = 256;
        
        if (rec.use_shared_memory) {
            rec.strategy = "Use shared memory for data reuse";
        } else if (rec.use_texture_memory) {
            rec.strategy = "Use texture memory for read-only data";
        } else if (rec.use_constant_memory) {
            rec.strategy = "Use constant memory for small read-only data";
        } else {
            rec.strategy = "Use global memory with coalesced access";
        }
        
        return rec;
    }
};

} // namespace dnn::infra
