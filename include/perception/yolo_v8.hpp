#pragma once
#include <vector>
#include <string>
namespace dnn::perception {
struct BoundingBox {
    float x, y, w, h;
    float confidence;
    std::string label;
};
class YOLOv8 {
public:
    YOLOv8() {}
    std::vector<BoundingBox> detect(const std::vector<float>& input) { return {}; }
};
} // namespace dnn::perception
