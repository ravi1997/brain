#include "reflex.hpp"
#include <algorithm>
#include <cctype>

Reflex::Reflex() {
    // Instincts (Hardcoded survival responses)
    keyword_responses["hello"] = "Greetings.";
    keyword_responses["hi"] = "Hello there.";
    keyword_responses["who are you"] = "I am a Neural Entity designed for learning.";
    keyword_responses["what are you"] = "I am a digital brain simulation.";
    keyword_responses["status"] = "SYSTEM ONLINE. Neural pathways active.";
    keyword_responses["help"] = "I learn from interaction. Teach me by saying 'Question | Answer'.";
    keyword_responses["time"] = "Time is a variable I am currently processing.";
    keyword_responses["what is"] = "I am not yet fully trained on that topic.";
    keyword_responses["why"] = "Causality is complex. I am still learning logic.";
    keyword_responses["exit"] = "Goodbye.";
}

std::string Reflex::get_reaction(const std::string& input) {
    std::string lower_input = input;
    std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), 
                   [](unsigned char c){ return std::tolower(c); });

    for (const auto& pair : keyword_responses) {
        if (contains(lower_input, pair.first)) {
            return pair.second;
        }
    }
    return ""; // No reflex found
}

bool Reflex::contains(const std::string& input, const std::string& key) {
    return input.find(key) != std::string::npos;
}
