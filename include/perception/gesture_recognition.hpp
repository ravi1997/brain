#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <limits>

namespace dnn::perception {

// Simple DTW-based gesture recognition
class GestureRecognition {
public:
    void add_template(const std::string& name, const std::vector<std::vector<float>>& sequence) {
        templates_[name] = sequence;
    }
    
    std::string recognize_gesture(const std::vector<std::vector<float>>& input) {
        if (templates_.empty()) return "";
        
       std::string best_match;
        float best_distance = std::numeric_limits<float>::max();
        
        for (const auto& [name, template_seq] : templates_) {
            float dist = dtw_distance(input, template_seq);
             if (dist < best_distance) {
                best_distance = dist;
                best_match = name;
            }
        }
        
        return best_match;
    }
    
private:
    std::unordered_map<std::string, std::vector<std::vector<float>>> templates_;
    
    float dtw_distance(const std::vector<std::vector<float>>& a, 
                      const std::vector<std::vector<float>>& b) {
        int n = a.size();
        int m = b.size();
        std::vector<std::vector<float>> dtw(n + 1, std::vector<float>(m + 1, std::numeric_limits<float>::max()));
        dtw[0][0] = 0;
        
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= m; j++) {
                float cost = euclidean(a[i-1], b[j-1]);
                dtw[i][j] = cost + std::min({dtw[i-1][j], dtw[i][j-1], dtw[i-1][j-1]});
            }
        }
        
        return dtw[n][m];
    }
    
    float euclidean(const std::vector<float>& a, const std::vector<float>& b) {
        float sum = 0;
        for (size_t i = 0; i < std::min(a.size(), b.size()); i++) {
            float diff = a[i] - b[i];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }
};

} // namespace dnn::perception
