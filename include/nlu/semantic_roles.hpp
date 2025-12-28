#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace dnn::nlu {

// Semantic Role Labeling (SRL)
class SemanticRoleLabeling {
public:
    struct SemanticRole {
        std::string role;  // ARG0, ARG1, etc.
        std::string text;
        int start_pos;
        int end_pos;
    };
    
    SemanticRoleLabeling() {}
    
    // Label semantic roles in a sentence
    std::vector<SemanticRole> label(const std::string& sentence) {
        std::vector<SemanticRole> roles;
        
        // Simple pattern-based SRL
        std::vector<std::string> words = split(sentence);
        
        // Find predicate (verb)
        int predicate_idx = find_predicate(words);
        if (predicate_idx < 0) return roles;
        
        // ARG0: Subject (before predicate)
        if (predicate_idx > 0) {
            SemanticRole arg0;
            arg0.role = "ARG0";
            arg0.text = words[predicate_idx - 1];
            arg0.start_pos = predicate_idx - 1;
            arg0.end_pos = predicate_idx - 1;
            roles.push_back(arg0);
        }
        
        // ARG1: Object (after predicate)
        if (predicate_idx < static_cast<int>(words.size()) - 1) {
            SemanticRole arg1;
            arg1.role = "ARG1";
            arg1.text = words[predicate_idx + 1];
            arg1.start_pos = predicate_idx + 1;
            arg1.end_pos = predicate_idx + 1;
            roles.push_back(arg1);
        }
        
        // Predicate itself
        SemanticRole pred;
        pred.role = "PREDICATE";
        pred.text = words[predicate_idx];
        pred.start_pos = predicate_idx;
        pred.end_pos = predicate_idx;
        roles.push_back(pred);
        
        return roles;
    }
    
private:
    std::vector<std::string> split(const std::string& text) {
        std::vector<std::string> words;
        std::string word;
        
        for (char c : text) {
            if (c == ' ' || c == '\n' || c == '\t' || c == '.' || c == ',') {
                if (!word.empty()) {
                    words.push_back(word);
                    word.clear();
                }
            } else {
                word += c;
            }
        }
        
        if (!word.empty()) {
            words.push_back(word);
        }
        
        return words;
    }
    
    int find_predicate(const std::vector<std::string>& words) {
        // Common verbs (simplified)
        std::vector<std::string> verbs = {
            "is", "are", "was", "were", "be", "been",
            "go", "went", "run", "ran", "eat", "ate",
            "see", "saw", "make", "made", "write", "wrote"
        };
        
        for (size_t i = 0; i < words.size(); i++) {
            std::string lower = to_lower(words[i]);
            for (const auto& verb : verbs) {
                if (lower == verb || lower == verb + "s" || lower == verb + "ing") {
                    return i;
                }
            }
        }
        
        return -1;
    }
    
    std::string to_lower(std::string s) {
        for (char& c : s) c = std::tolower(c);
        return s;
    }
};

} // namespace dnn::nlu
