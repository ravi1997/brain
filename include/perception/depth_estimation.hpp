#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace dnn::perception {

// Monocular depth estimation
class DepthEstimation {
public:
    DepthEstimation(int width = 640, int height = 480) 
        : width_(width), height_(height) {}
    
    // Estimate depth map from single image
    std::vector<float> estimate_depth(const std::vector<float>& image) {
        std::vector<float> depth_map(width_ * height_);
        
        // Simple gradient-based depth estimation
        for (int y = 1; y < height_ - 1; y++) {
            for (int x = 1; x < width_ - 1; x++) {
                int idx = y * width_ + x;
                
                // Compute gradient magnitude
                float grad_x = image[idx + 1] - image[idx - 1];
                float grad_y = image[(y + 1) * width_ + x] - image[(y - 1) * width_ + x];
                float gradient = std::sqrt(grad_x * grad_x + grad_y * grad_y);
                
                // Inverse gradient as depth proxy (sharper = closer)
                depth_map[idx] = 1.0f / (gradient + 0.1f);
            }
        }
        
        // Normalize
        float max_depth = *std::max_element(depth_map.begin(), depth_map.end());
        if (max_depth > 0) {
            for (auto& d : depth_map) {
                d /= max_depth;
            }
        }
        
        return depth_map;
    }
    
private:
    int width_;
    int height_;
};

} // namespace dnn::perception
