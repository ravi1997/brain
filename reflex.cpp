#include "reflex.hpp"
#include <algorithm>
#include <cctype>

Reflex::Reflex() {
    // Instincts (Hardcoded survival responses)
    keyword_responses["hello"] = {{"Greetings.", 1.0}, {"Hello there.", 1.0}};
    keyword_responses["status"] = {{"SYSTEM ONLINE.", 1.0}, {"Neural pathways active.", 1.0}};
    keyword_responses["help"] = {{"I learn from interaction.", 1.0}, {"Say 'Question | Answer' to teach me.", 1.0}};
    keyword_responses["exit"] = {{"Goodbye.", 1.0}};
}

std::string Reflex::get_reaction(const std::string& input) {
    std::string lower_input = input;
    std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), 
                   [](unsigned char c){ return std::tolower(c); });

    for (auto& pair : keyword_responses) {
        if (contains(lower_input, pair.first)) {
            // Pick based on weights
            auto& choices = pair.second;
            double total_weight = 0;
            for (auto& c : choices) total_weight += c.weight;
            
            double r = static_cast<double>(rand()) / RAND_MAX * total_weight;
            double cumulative = 0;
            for (auto& c : choices) {
                cumulative += c.weight;
                if (r <= cumulative) return c.text;
            }
            return choices[0].text;
        }
    }
    return ""; // No reflex found
}

void Reflex::reinforce(const std::string& keyword, const std::string& response, double reward) {
    if (keyword_responses.count(keyword)) {
        for (auto& choice : keyword_responses[keyword]) {
            if (choice.text == response) {
                choice.weight = std::max(0.1, choice.weight + reward);
                return;
            }
        }
    }
}

bool Reflex::contains(const std::string& input, const std::string& key) {
    return input.find(key) != std::string::npos;
}
