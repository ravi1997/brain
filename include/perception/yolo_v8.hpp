#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace dnn::perception {

struct BoundingBox {
    float x, y, w, h;    // Center coordinates, width, height
    float confidence;
    int class_id;
    std::string class_name;
    
    BoundingBox() : x(0), y(0), w(0), h(0), confidence(0), class_id(0) {}
    BoundingBox(float cx, float cy, float width, float height, float conf, int cls)
        : x(cx), y(cy), w(width), h(height), confidence(conf), class_id(cls) {}
};

// YOLOv8 object detection
class YOLOv8 {
public:
    YOLOv8(int input_width = 640, int input_height = 640, float conf_threshold = 0.25f, 
           float iou_threshold = 0.45f)
        : input_width_(input_width), input_height_(input_height),
          conf_threshold_(conf_threshold), iou_threshold_(iou_threshold) {
        
        // Initialize COCO class names (subset for demonstration)
        class_names_ = {
            "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train",
            "truck", "boat", "traffic light", "fire hydrant", "stop sign",
            "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep",
            "cow", "elephant", "bear", "zebra", "giraffe", "backpack"
        };
    }
    
    // Detect objects in image
    std::vector<BoundingBox> detect(const std::vector<float>& image) {
        std::vector<BoundingBox> detections;
        
        // Simplified YOLOv8 detection (simulated grid-based detection)
        int grid_size = 20;  // 20x20 grid
        float cell_width = static_cast<float>(input_width_) / grid_size;
        float cell_height = static_cast<float>(input_height_) / grid_size;
        
        // Simulate detection at each grid cell
        for (int gy = 0; gy < grid_size; gy++) {
            for (int gx = 0; gx < grid_size; gx++) {
                // Sample image features at this location
                int img_idx = (gy * grid_size + gx) % image.size();
                float feature = image.empty() ? 0.5f : std::abs(image[img_idx]);
                
                // Threshold-based detection simulation
                if (feature > 0.6f) {
                    float cx = (gx + 0.5f) * cell_width;
                    float cy = (gy + 0.5f) * cell_height;
                    float w = cell_width * 2.0f;
                    float h = cell_height * 2.0f;
                    float conf = feature;
                    int cls = static_cast<int>(feature * class_names_.size()) % class_names_.size();
                    
                    BoundingBox bbox(cx, cy, w, h, conf, cls);
                    bbox.class_name = class_names_[cls];
                    detections.push_back(bbox);
                }
            }
        }
        
        // Apply NMS (Non-Maximum Suppression)
        detections = non_max_suppression(detections);
        
        return detections;
    }
    
    // Non-Maximum Suppression
    std::vector<BoundingBox> non_max_suppression(const std::vector<BoundingBox>& boxes) {
        if (boxes.empty()) return {};
        
        // Sort by confidence
        std::vector<BoundingBox> sorted_boxes = boxes;
        std::sort(sorted_boxes.begin(), sorted_boxes.end(),
                 [](const BoundingBox& a, const BoundingBox& b) {
                     return a.confidence > b.confidence;
                 });
        
        std::vector<BoundingBox> result;
        std::vector<bool> suppressed(sorted_boxes.size(), false);
        
        for (size_t i = 0; i < sorted_boxes.size(); i++) {
            if (suppressed[i]) continue;
            
            result.push_back(sorted_boxes[i]);
            
            // Suppress overlapping boxes
            for (size_t j = i + 1; j < sorted_boxes.size(); j++) {
                if (suppressed[j]) continue;
                
                float iou = calculate_iou(sorted_boxes[i], sorted_boxes[j]);
                if (iou > iou_threshold_) {
                    suppressed[j] = true;
                }
            }
        }
        
        return result;
    }
    
    // Calculate IoU (Intersection over Union)
    float calculate_iou(const BoundingBox& a, const BoundingBox& b) {
        // Convert center format to corner format
        float a_x1 = a.x - a.w / 2.0f;
        float a_y1 = a.y - a.h / 2.0f;
        float a_x2 = a.x + a.w / 2.0f;
        float a_y2 = a.y + a.h / 2.0f;
        
        float b_x1 = b.x - b.w / 2.0f;
        float b_y1 = b.y - b.h / 2.0f;
        float b_x2 = b.x + b.w / 2.0f;
        float b_y2 = b.y + b.h / 2.0f;
        
        // Calculate intersection
        float inter_x1 = std::max(a_x1, b_x1);
        float inter_y1 = std::max(a_y1, b_y1);
        float inter_x2 = std::min(a_x2, b_x2);
        float inter_y2 = std::min(a_y2, b_y2);
        
        if (inter_x2 < inter_x1 || inter_y2 < inter_y1) {
            return 0.0f;  // No intersection
        }
        
        float inter_area = (inter_x2 - inter_x1) * (inter_y2 - inter_y1);
        float a_area = a.w * a.h;
        float b_area = b.w * b.h;
        float union_area = a_area + b_area - inter_area;
        
        return union_area > 0 ? inter_area / union_area : 0.0f;
    }
    
    // Set confidence threshold
    void set_conf_threshold(float threshold) {
        conf_threshold_ = threshold;
    }
    
    // Set IoU threshold for NMS
    void set_iou_threshold(float threshold) {
        iou_threshold_ = threshold;
    }
    
private:
    int input_width_;
    int input_height_;
    float conf_threshold_;
    float iou_threshold_;
    std::vector<std::string> class_names_;
};

} // namespace dnn::perception
