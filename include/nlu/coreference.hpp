#pragma once
#include <vector>
#include <string>
#include <unordered_map>

namespace dnn::nlu {

// Rule-based coreference resolution
class CoreferenceResolution {
public:
    std::vector<std::string> resolve(const std::vector<std::string>& tokens) {
        std::vector<std::string> resolved = tokens;
        std::string last_noun;
        
        for (size_t i = 0; i < tokens.size(); i++) {
            std::string lower = to_lower(tokens[i]);
            if (is_noun(lower)) {
                last_noun = tokens[i];
            } else if (is_pronoun(lower) && !last_noun.empty()) {
                resolved[i] = last_noun;
            }
        }
        return resolved;
    }
    
private:
    bool is_pronoun(const std::string& word) const {
        return word == "he" || word == "she" || word == "it" ||  word == "they" || word == "him" || word == "her";
    }
    
    bool is_noun(const std::string& word) const {
        return word.length() > 2 && word[0] >= 'A' && word[0] <= 'Z';
    }
    
    std::string to_lower(std::string s) const {
        for (char& c : s) c = tolower(c);
        return s;
    }
};

} // namespace dnn::nlu
