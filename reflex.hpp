#pragma once
#include <string>
#include <vector>
#include <map>

class Reflex {
public:
    Reflex();
    std::string get_reaction(const std::string& input);

private:
    std::map<std::string, std::string> keyword_responses;
    bool contains(const std::string& input, const std::string& key);
};
