#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace dnn::perception {

// Visual Question Answering (VQA) - combining vision and language
class VisualQuestionAnswering {
public:
    struct VisualFeature {
        std::string object_name;
        std::vector<float> bbox;  // x, y, w, h
        std::vector<float> features;  // Visual features
        float confidence;
        
        VisualFeature() : confidence(0) {}
    };
    
    struct Question {
        std::string text;
        std::string question_type;  // "what", "where", "how many", "is", "color"
        std::vector<std::string> keywords;
        
        Question() {}
        Question(const std::string& t) : text(t) {
            question_type = classify_question(t);
            keywords = extract_keywords(t);
        }
        
    private:
        std::string classify_question(const std::string& text) const {
            std::string lower = to_lower(text);
            
            if (lower.find("what") == 0) return "what";
            if (lower.find("where") == 0) return "where";
            if (lower.find("how many") == 0) return "count";
            if (lower.find("is ") == 0 || lower.find("are ") == 0) return "verify";
            if (lower.find("color") != std::string::npos) return "color";
            if (lower.find("who") == 0) return "who";
            
            return "what";
        }
        
        std::vector<std::string> extract_keywords(const std::string& text) const {
            std::vector<std::string> keywords;
            std::string word;
            
            for (char c : text) {
                if (std::isalnum(c)) {
                    word += std::tolower(c);
                } else if (!word.empty()) {
                    // Skip common stop words
                    if (word != "the" && word != "a" && word != "is" && 
                        word != "are" && word != "in" && word != "on") {
                        keywords.push_back(word);
                    }
                    word.clear();
                }
            }
            
            if (!word.empty()) {
                keywords.push_back(word);
            }
            
            return keywords;
        }
        
        std::string to_lower(std::string s) const {
            for (char& c : s) c = std::tolower(c);
            return s;
        }
    };
    
    VisualQuestionAnswering() {
        // Initialize knowledge base
        initialize_knowledge();
    }
    
    // Answer question based on visual features
    std::string answer(const Question& question, 
                      const std::vector<VisualFeature>& visual_features) {
        if (question.question_type == "what") {
            return answer_what(question, visual_features);
        } else if (question.question_type == "where") {
            return answer_where(question, visual_features);
        } else if (question.question_type == "count") {
            return answer_count(question, visual_features);
        } else if (question.question_type == "verify") {
            return answer_verify(question, visual_features);
        } else if (question.question_type == "color") {
            return answer_color(question, visual_features);
        }
        
        return "I don't know";
    }
    
private:
    std::unordered_map<std::string, std::vector<std::string>> object_attributes_;
    std::unordered_map<std::string, std::string> color_map_;
    
    void initialize_knowledge() {
        // Object attributes
        object_attributes_["person"] = {"standing", "sitting", "walking", "running"};
        object_attributes_["car"] = {"parked", "moving", "red", "blue", "black"};
        object_attributes_["dog"] = {"sitting", "running", "brown", "black", "white"};
        
        // Color knowledge
        color_map_["person"] = "various";
        color_map_["car"] = "red";
        color_map_["dog"] = "brown";
        color_map_["tree"] = "green";
        color_map_["sky"] = "blue";
    }
    
    std::string answer_what(const Question& q, const std::vector<VisualFeature>& features) {
        // Find most relevant object based on keywords
        float best_score = 0.0f;
        std::string best_object;
        
        for (const auto& feature : features) {
            float score = compute_relevance(q.keywords, feature);
            if (score > best_score) {
                best_score = score;
                best_object = feature.object_name;
            }
        }
        
        if (!best_object.empty()) {
            return "a " + best_object;
        }
        
        return "unknown object";
    }
    
    std::string answer_where(const Question& q, const std::vector<VisualFeature>& features) {
        // Find object mentioned in question
        for (const auto& keyword : q.keywords) {
            for (const auto& feature : features) {
                if (feature.object_name.find(keyword) != std::string::npos) {
                    // Determine location based on bbox
                    if (feature.bbox.size() >= 4) {
                        float cx = feature.bbox[0] + feature.bbox[2] / 2;
                        float cy = feature.bbox[1] + feature.bbox[3] / 2;
                        
                        std::string location;
                        if (cx < 0.33f) location = "on the left";
                        else if (cx > 0.67f) location = "on the right";
                        else location = "in the center";
                        
                        if (cy < 0.33f) location += " at the top";
                        else if (cy > 0.67f) location += " at the bottom";
                        
                        return location;
                    }
                }
            }
        }
        
        return "location unknown";
    }
    
    std::string answer_count(const Question& q, const std::vector<VisualFeature>& features) {
        // Count objects matching keywords
        int count = 0;
        
        for (const auto& keyword : q.keywords) {
            for (const auto& feature : features) {
                if (feature.object_name.find(keyword) != std::string::npos) {
                    count++;
                }
            }
        }
        
        return std::to_string(count);
    }
    
    std::string answer_verify(const Question& q, const std::vector<VisualFeature>& features) {
        // Check if object exists
        for (const auto& keyword : q.keywords) {
            for (const auto& feature : features) {
                if (feature.object_name.find(keyword) != std::string::npos) {
                    return "yes";
                }
            }
        }
        
        return "no";
    }
    
    std::string answer_color(const Question& q, const std::vector<VisualFeature>& features) {
        // Find object and return its color
        for (const auto& keyword : q.keywords) {
            for (const auto& feature : features) {
                if (feature.object_name.find(keyword) != std::string::npos) {
                    if (color_map_.count(feature.object_name)) {
                        return color_map_[feature.object_name];
                    }
                }
            }
        }
        
        return "color unknown";
    }
    
    float compute_relevance(const std::vector<std::string>& keywords,
                           const VisualFeature& feature) const {
        float score = 0.0f;
        
        for (const auto& keyword : keywords) {
            if (feature.object_name.find(keyword) != std::string::npos) {
                score += 1.0f;
            }
        }
        
        score *= feature.confidence;
        return score;
    }
};

} // namespace dnn::perception
