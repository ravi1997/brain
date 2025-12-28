#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace dnn::nlu {

// Dialogue State Tracking for conversational AI
class DialogueStateTracking {
public:
    struct DialogueState {
        std::string intent;
        std::unordered_map<std::string, std::string> slots;
        std::string current_topic;
        int turn_count;
        
        DialogueState() : turn_count(0) {}
    };
    
    DialogueStateTracking() {}
    
    // Update state based on user utterance
    void update(const std::string& utterance) {
        state_.turn_count++;
        
        // Intent detection (simple keyword matching)
        std::string lower = to_lower(utterance);
        
        if (lower.find("book") != std::string::npos || 
            lower.find("reserve") != std::string::npos) {
            state_.intent = "booking";
            state_.current_topic = "reservation";
        } else if (lower.find("cancel") != std::string::npos) {
            state_.intent = "cancellation";
        } else if (lower.find("question") != std::string::npos ||
                  lower.find("what") != std::string::npos ||
                  lower.find("how") != std::string::npos) {
            state_.intent = "question";
        } else {
            state_.intent = "informational";
        }
        
        // Slot extraction (simple pattern matching)
        extract_slots(utterance);
    }
    
    // Get current state
    DialogueState get_state() const {
        return state_;
    }
    
    // Reset state
    void reset() {
        state_ = DialogueState();
    }
    
    // Get specific slot value
    std::string get_slot(const std::string& slot_name) const {
        auto it = state_.slots.find(slot_name);
        return it != state_.slots.end() ? it->second : "";
    }
    
private:
    DialogueState state_;
    
    void extract_slots(const std::string& utterance) {
        // Extract date (simple pattern: contains digits)
        if (utterance.find_first_of("0123456789") != std::string::npos) {
            state_.slots["date"] = "extracted_date";
        }
        
        // Extract location (contains "in" or "at")
        size_t pos = utterance.find(" in ");
        if (pos == std::string::npos) pos = utterance.find(" at ");
        
        if (pos != std::string::npos) {
            std::string location = utterance.substr(pos + 4);
            if (location.size() > 0) {
                state_.slots["location"] = location.substr(0, location.find(" "));
            }
        }
    }
    
    std::string to_lower(std::string s) {
        for (char& c : s) c = std::tolower(c);
        return s;
    }
};

} // namespace dnn::nlu
