#pragma once
#include <vector>
#include <cmath>

namespace dnn::perception {

// Optical Flow - Lucas-Kanade method for motion estimation
class OpticalFlow {
public:
    struct FlowVector {
        float dx, dy;  // Motion in x and y
        float magnitude;
        
        FlowVector() : dx(0), dy(0), magnitude(0) {}
        FlowVector(float x, float y) : dx(x), dy(y) {
            magnitude = std::sqrt(dx*dx + dy*dy);
        }
    };
    
    OpticalFlow(int width, int height, int window_size = 5)
        : width_(width), height_(height), window_size_(window_size) {}
    
    // Compute optical flow between two frames
    std::vector<FlowVector> compute_flow(const std::vector<float>& frame1,
                                         const std::vector<float>& frame2) {
        std::vector<FlowVector> flow(width_ * height_);
        
        int half_window = window_size_ / 2;
        
        // For each pixel
        for (int y = half_window; y < height_ - half_window; y++) {
            for (int x = half_window; x < width_ - half_window; x++) {
                flow[y * width_ + x] = lucas_kanade(frame1, frame2, x, y);
            }
        }
        
        return flow;
    }
    
private:
    int width_;
    int height_;
    int window_size_;
    
    FlowVector lucas_kanade(const std::vector<float>& img1,
                            const std::vector<float>& img2,
                            int cx, int cy) {
        int half = window_size_ / 2;
        
        // Compute image gradients and temporal derivative
        float sum_Ix2 = 0, sum_Iy2 = 0, sum_IxIy = 0;
        float sum_IxIt = 0, sum_IyIt = 0;
        
        for (int dy = -half; dy <= half; dy++) {
            for (int dx = -half; dx <= half; dx++) {
                int x = cx + dx;
                int y = cy + dy;
                
                if (x <= 0 || x >= width_-1 || y <= 0 || y >= height_-1) {
                    continue;
                }
                
                // Spatial gradients (Sobel-like)
                float Ix = (get_pixel(img1, x+1, y) - get_pixel(img1, x-1, y)) / 2.0f;
                float Iy = (get_pixel(img1, x, y+1) - get_pixel(img1, x, y-1)) / 2.0f;
                
                // Temporal derivative
                float It = get_pixel(img2, x, y) - get_pixel(img1, x, y);
                
                sum_Ix2 += Ix * Ix;
                sum_Iy2 += Iy * Iy;
                sum_IxIy += Ix * Iy;
                sum_IxIt += Ix * It;
                sum_IyIt += Iy * It;
            }
        }
        
        // Solve 2x2 system: [Ix^2, IxIy; IxIy, Iy^2] * [u; v]= -[IxIt; IyIt]
        float det = sum_Ix2 * sum_Iy2 - sum_IxIy * sum_IxIy;
        
        if (std::abs(det) < 1e-6f) {
            return FlowVector(0, 0);
        }
        
        float u = -(sum_Iy2 * sum_IxIt - sum_IxIy * sum_IyIt) / det;
        float v = -(sum_Ix2 * sum_IyIt - sum_IxIy * sum_IxIt) / det;
        
        return FlowVector(u, v);
    }
    
    float get_pixel(const std::vector<float>& img, int x, int y) const {
        int idx = y * width_ + x;
        return (idx >= 0 && idx < static_cast<int>(img.size())) ? img[idx] : 0.0f;
    }
};

} // namespace dnn::perception
