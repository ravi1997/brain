#pragma once
#include <vector>
#include <string>

namespace dnn::perception {

// Image captioning - generate text descriptions of images
class ImageCaptioning {
public:
    ImageCaptioning() {
        // Initialize vocabulary
        vocab_ = {"a", "the", "dog", "cat", "person", "car", "tree", "building",
                 "is", "sitting", "standing", "running", "walking",
                 "on", "in", "near", "with"};
    }
    
    // Generate caption from image features
    std::string generate_caption(const std::vector<float>& image_features) {
        if (image_features.empty()) {
            return "An image.";
        }
        
        // Simple template-based caption generation
        std::string caption;
        
        // Determine main object based on feature statistics
        float mean = 0.0f;
        for (float f : image_features) {
            mean += f;
        }
        mean /= image_features.size();
        
        if (mean > 0.7f) {
            caption = "A person standing in a scene.";
        } else if (mean > 0.5f) {
            caption = "A car on the road.";
        } else if (mean > 0.3f) {
            caption = "A dog sitting on the grass.";
        } else {
            caption = "A building with trees nearby.";
        }
        
        return caption;
    }
    
private:
    std::vector<std::string> vocab_;
};

} // namespace dnn::perception
