#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>
#include "../include/json.hpp"

namespace research_tools {

    using json = nlohmann::json;

    struct ResearchResult {
        std::string summary;
        std::vector<std::string> related_topics;
    };

    inline std::string sanitize_topic(const std::string& topic) {
        std::string result = "";
        for (char c : topic) {
            if (c == ' ') result += "_";
            else if (c == '+') result += "%2B";
            else result += c;
        }
        return result;
    }

    inline ResearchResult fetch_comprehensive(const std::string& topic) {
        std::string safe_topic = sanitize_topic(topic);
        // Fetch extracts AND links
        std::string url = "https://en.wikipedia.org/w/api.php?action=query&format=json&prop=extracts|links&exintro&explaintext&pllimit=20&titles=" + safe_topic;
        
        std::string tmp_file = "/tmp/brain_research.json";
        std::string command = "curl -s \"" + url + "\" -o " + tmp_file;
        
        int ret = std::system(command.c_str());
        if (ret != 0) return {"Connection Failed", {}};

        std::ifstream f(tmp_file);
        if (!f.is_open()) return {"Read Failed", {}};
        
        ResearchResult result;
        try {
            json j;
            f >> j;
            
            auto pages = j["query"]["pages"];
            for (auto& [key, value] : pages.items()) {
                if (key == "-1") continue; // Missing
                
                if (value.contains("extract")) {
                    result.summary = value["extract"].get<std::string>();
                }
                
                if (value.contains("links")) {
                    for (const auto& link : value["links"]) {
                        result.related_topics.push_back(link["title"].get<std::string>());
                    }
                }
            }
        } catch (...) {
            result.summary = "Parse Error";
        }
        
        if (result.summary.empty()) result.summary = "No information found on topic: " + topic;
        return result;
    }
    
    // Legacy support
    inline std::string fetch_summary(const std::string& topic) {
        return fetch_comprehensive(topic).summary;
    }
}
