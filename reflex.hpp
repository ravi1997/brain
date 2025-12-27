#pragma once
#include <string>
#include <vector>
#include <map>

class Reflex {
public:
    Reflex();
    std::string get_reaction(const std::string& input);
    void reinforce(const std::string& keyword, const std::string& response, double reward);
    
    // Persistence
    void save(const std::string& filename);
    void load(const std::string& filename);

    struct WeightedResponse {
        std::string text;
        double weight;
    };
    std::map<std::string, std::vector<WeightedResponse>>& get_instincts() { return keyword_responses; }

private:
    std::map<std::string, std::vector<WeightedResponse>> keyword_responses;
    bool contains(const std::string& input, const std::string& key);
};
