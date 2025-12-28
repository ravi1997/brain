#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace dnn::nlu {

// Metaphor understanding and interpretation
class MetaphorUnderstanding {
public:
    MetaphorUnderstanding() {
        // Initialize metaphor mappings
        metaphors_["time is money"] = "Time is a valuable resource";
        metaphors_["life is a journey"] = "Life is a progression with milestones";
        metaphors_["ideas are food"] = "Ideas can be consumed and digested";
        metaphors_["argument is war"] = "Arguments are combative exchanges";
        metaphors_["love is a journey"] = "Love experiences ups and downs";
        metaphors_["heart"] = "emotions/feelings";
        metaphors_["light"] = "understanding/knowledge";
        metaphors_["darkness"] = "ignorance/confusion";
    }
    
    // Detect if text contains metaphor
    bool is_metaphorical(const std::string& text) {
        std::string lower = to_lower(text);
        
        // Check for known metaphorical phrases
        for (const auto& [metaphor, _] : metaphors_) {
            if (lower.find(metaphor) != std::string::npos) {
                return true;
            }
        }
        
        // Check for metaphor indicators
        if (lower.find(" like ") != std::string::npos ||
            lower.find(" as ") != std::string::npos ||
            lower.find("metaphorically") != std::string::npos) {
            return true;
        }
        
        return false;
    }
    
    // Interpret metaphor
    std::string interpret(const std::string& text) {
        std::string lower = to_lower(text);
        
        // Find matching metaphor
        for (const auto& [metaphor, interpretation] : metaphors_) {
            if (lower.find(metaphor) != std::string::npos) {
                return interpretation;
            }
        }
        
        return "Literal meaning: " + text;
    }
    
    // Extract metaphorical components
    struct MetaphorComponents {
        std::string source_domain;
        std::string target_domain;
        std::string mapping;
    };
    
    MetaphorComponents analyze(const std::string& text) {
        MetaphorComponents components;
        std::string lower = to_lower(text);
        
        // Simple pattern matching for "X is Y" structure
        size_t is_pos = lower.find(" is ");
        if (is_pos != std::string::npos) {
            components.target_domain = text.substr(0, is_pos);
            size_t rest_start = is_pos + 4;
            components.source_domain = text.substr(rest_start);
            components.mapping = "conceptual blending";
        }
        
        return components;
    }
    
private:
    std::unordered_map<std::string, std::string> metaphors_;
    
    std::string to_lower(std::string s) {
        for (char& c : s) c = std::tolower(c);
        return s;
    }
};

} // namespace dnn::nlu
