#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace dnn::nlu {

// Empathetic response generation
class EmpatheticResponse {
public:
    EmpatheticResponse() {
        // Initialize emotion-response mappings
        responses_["sad"] = {"I'm sorry to hear that.", "That must be difficult for you.", 
                            "I understand how that could be upsetting."};
        responses_["happy"] = {"That's wonderful!", "I'm so glad to hear that!", 
                              "How exciting for you!"};
        responses_["angry"] = {"I can understand your frustration.", "That sounds really frustrating.",
                              "I see why you'd feel that way."};
        responses_["anxious"] = {"That's a valid concern.", "It's natural to feel worried about that.",
                                "I hear your concerns."};
        responses_["neutral"] = {"I see.", "Tell me more about that.", "I understand."};
    }
    
    // Generate empathetic response based on detected emotion
    std::string generate(const std::string& user_input, const std::string& detected_emotion = "") {
        std::string emotion = detected_emotion.empty() ? detect_emotion(user_input) : detected_emotion;
        
        auto it = responses_.find(emotion);
        if (it == responses_.end()) {
            it = responses_.find("neutral");
        }
        
        // Select first response (could be randomized)
        if (!it->second.empty()) {
            return it->second[0];
        }
        
        return "I understand.";
    }
    
    // Detect emotion from text (simple keyword matching)
    std::string detect_emotion(const std::string& text) const {
        std::string lower = to_lower(text);
        
        // Sad keywords
        if (lower.find("sad") != std::string::npos ||
            lower.find("depressed") != std::string::npos ||
            lower.find("upset") != std::string::npos ||
            lower.find("cry") != std::string::npos) {
            return "sad";
        }
        
        // Happy keywords
        if (lower.find("happy") != std::string::npos ||
            lower.find("excited") != std::string::npos ||
            lower.find("great") != std::string::npos ||
            lower.find("wonderful") != std::string::npos) {
            return "happy";
        }
        
        // Angry keywords
        if (lower.find("angry") != std::string::npos ||
            lower.find("furious") != std::string::npos ||
            lower.find("frustrated") != std::string::npos ||
            lower.find("mad") != std::string::npos) {
            return "angry";
        }
        
        // Anxious keywords
        if (lower.find("worried") != std::string::npos ||
            lower.find("anxious") != std::string::npos ||
            lower.find("nervous") != std::string::npos ||
            lower.find("concerned") != std::string::npos) {
            return "anxious";
        }
        
        return "neutral";
    }
    
private:
    std::unordered_map<std::string, std::vector<std::string>> responses_;
    
    std::string to_lower(std::string s) const {
        for (char& c : s) c = std::tolower(c);
        return s;
    }
};

} // namespace dnn::nlu
