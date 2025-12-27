#include "reflex.hpp"
#include "json.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

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
        // Check exact containment OR fuzzy match
        if (contains(lower_input, pair.first) || fuzzy_match(lower_input) == pair.first) {
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

// Helper: Levenshtein Distance
size_t levenshtein_distance(const std::string& s1, const std::string& s2) {
    const size_t m = s1.length();
    const size_t n = s2.length();
    if (m == 0) return n;
    if (n == 0) return m;

    std::vector<size_t> row(n + 1);
    std::iota(row.begin(), row.end(), 0);

    for (size_t i = 1; i <= m; ++i) {
        row[0] = i;
        size_t prev = i - 1;
        for (size_t j = 1; j <= n; ++j) {
            size_t temp = row[j];
            if (s1[i - 1] == s2[j - 1]) {
                row[j] = prev;
            } else {
                row[j] = std::min({ row[j - 1], row[j], prev }) + 1;
            }
            prev = temp;
        }
    }
    return row[n];
}

std::string Reflex::fuzzy_match(const std::string& input) {
    // Mega-Batch 6: Fuzzy Match Logic using Levenshtein Distance
    
    std::string best_match;
    size_t min_dist = 1000;
    
    // Check whole input first (single word scenario)
    for (const auto& [key, val] : keyword_responses) {
        if (key.length() <= 3) continue; // Skip short keys to avoid false positives
        size_t dist = levenshtein_distance(input, key);
        if (dist <= 1 && dist < min_dist) {
            min_dist = dist;
            best_match = key;
        }
    }
    if (!best_match.empty()) return best_match;

    // Split input into words and check each
    std::string word;
    for (size_t i = 0; i < input.length(); ++i) {
        if (std::isalpha(input[i])) {
            word += input[i];
        } else {
            if (!word.empty() && word.length() > 3) {
                for (const auto& [key, val] : keyword_responses) {
                    if (key.length() <= 3) continue;
                    size_t dist = levenshtein_distance(word, key);
                    if (dist <= 1) return key; // Return immediate match for efficiency
                }
            }
            word = "";
        }
    }
    // Check last word
    if (!word.empty() && word.length() > 3) {
        for (const auto& [key, val] : keyword_responses) {
             if (key.length() <= 3) continue;
             size_t dist = levenshtein_distance(word, key);
             if (dist <= 1) return key;
        }
    }
    return "";
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

void Reflex::save(const std::string& filename) {
    json j;
    for (const auto& [keyword, responses] : keyword_responses) {
        json j_responses = json::array();
        for (const auto& r : responses) {
            j_responses.push_back({{"text", r.text}, {"weight", r.weight}});
        }
        j[keyword] = j_responses;
    }
    std::ofstream o(filename);
    o << j.dump(4);
}

void Reflex::load(const std::string& filename) {
    std::ifstream i(filename);
    if (!i.is_open()) return;
    json j;
    try {
        i >> j;
        for (auto& [keyword, j_responses] : j.items()) {
            std::vector<WeightedResponse> responses;
            for (const auto& item : j_responses) {
                responses.push_back({item["text"], item["weight"]});
            }
            if (!responses.empty()) {
                keyword_responses[keyword] = responses;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Reflex load error: " << e.what() << std::endl;
    }
}
