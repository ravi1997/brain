#pragma once
#include <string>
#include <vector>

namespace dnn::nlu {

// Sarcasm detection
class SarcasmDetection {
public:
    SarcasmDetection() {}
    
    // Detect if text is sarcastic
    bool is_sarcastic(const std::string& text) {
        std::string lower = to_lower(text);
        
        // Check for sarcasm indicators
        int sarcasm_score = 0;
        
        // Extreme positive words in negative context
        if ((lower.find("great") != std::string::npos ||
             lower.find("wonderful") != std::string::npos ||
             lower.find("perfect") != std::string::npos) &&
            (lower.find("not") != std::string::npos ||
             lower.find("just") != std::string::npos ||
             lower.find("really") != std::string::npos)) {
            sarcasm_score += 2;
        }
        
        // Obvious exaggeration markers
        if (lower.find("oh ") != std::string::npos ||
            lower.find("wow") != std::string::npos ||
            lower.find("yeah right") != std::string::npos ||
            lower.find("sure") != std::string::npos) {
            sarcasm_score += 1;
        }
        
        // Punctuation patterns (!!! or ...)
        if (lower.find("!!!") != std::string::npos ||
            lower.find("...") != std::string::npos) {
            sarcasm_score += 1;
        }
        
        // Contradiction indicators
        if (lower.find("but") != std::string::npos ||
            lower.find("however") != std::string::npos) {
            sarcasm_score += 1;
        }
        
        return sarcasm_score >= 2;
    }
    
    // Get sarcasm confidence score (0-1)
    float get_confidence(const std::string& text) {
        int features = count_sarcasm_features(text);
        return std::min(1.0f, features / 5.0f);
    }
    
private:
    int count_sarcasm_features(const std::string& text) {
        std::string lower = to_lower(text);
        int count = 0;
        
        if (lower.find("great") != std::string::npos ||
            lower.find("perfect") != std::string::npos) count++;
        if (lower.find("oh ") != std::string::npos) count++;
        if (lower.find("!!!") != std::string::npos) count++;
        if (lower.find("yeah right") != std::string::npos) count += 2;
        if (lower.find("not") != std::string::npos) count++;
        
       return count;
    }
    
    std::string to_lower(std::string s) {
        for (char& c : s) c = std::tolower(c);
        return s;
    }
};

} // namespace dnn::nlu
