#pragma once
#include <vector>
#include <cmath>

namespace dnn::perception {

// Semantic segmentation - pixel-wise classification
class SemanticSegmentation {
public:
    enum class SegmentClass {
        BACKGROUND = 0,
        PERSON = 1,
        CAR = 2,
        ROAD = 3,
        BUILDING = 4,
        SKY = 5,
        VEGETATION = 6
    };
    
    SemanticSegmentation(int width = 640, int height = 480)
        : width_(width), height_(height) {}
    
    // Segment image into semantic classes
    std::vector<int> segment(const std::vector<float>& image) {
        std::vector<int> segmentation_map(width_ * height_);
        
        // Simple threshold-based segmentation
        for (int i = 0; i < width_ * height_; i++) {
            if (i >= static_cast<int>(image.size())) break;
            
            float intensity = image[i];
            
            // Classify based on intensity
            if (intensity > 0.9f) {
                segmentation_map[i] = static_cast<int>(SegmentClass::SKY);
            } else if (intensity > 0.7f) {
                segmentation_map[i] = static_cast<int>(SegmentClass::BUILDING);
            } else if (intensity > 0.5f) {
                segmentation_map[i] = static_cast<int>(SegmentClass::PERSON);
            } else if (intensity > 0.3f) {
                segmentation_map[i] = static_cast<int>(SegmentClass::CAR);
            } else if (intensity > 0.15f) {
                segmentation_map[i] = static_cast<int>(SegmentClass::VEGETATION);
            } else {
                segmentation_map[i] = static_cast<int>(SegmentClass::ROAD);
            }
        }
        
        return segmentation_map;
    }
    
    // Get class distribution
    std::vector<float> get_class_distribution(const std::vector<int>& segmentation) {
        std::vector<float> distribution(7, 0.0f);
        
        for (int cls : segmentation) {
            if (cls >= 0 && cls < 7) {
                distribution[cls] += 1.0f;
            }
        }
        
        // Normalize
        float total = segmentation.size();
        for (auto& d : distribution) {
            d /= total;
        }
        
        return distribution;
    }
    
private:
    int width_;
    int height_;
};

} // namespace dnn::perception
