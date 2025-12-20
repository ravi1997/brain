#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>

namespace research_tools {

    inline std::string sanitize_topic(const std::string& topic) {
        std::string result = "";
        for (char c : topic) {
            if (c == ' ') result += "_";
            else if (c == '+') result += "%2B";
            else result += c;
        }
        return result;
    }

    inline std::string parse_wikipedia_json(const std::string& json_content) {
        // Very basic "parser" to find "extract":"..."
        // Limitations: escaped characters etc.
        
        std::string key = "\"extract\":\"";
        size_t start_pos = json_content.find(key);
        if (start_pos == std::string::npos) return "";
        start_pos += key.length();

        std::string result = "";
        bool escape = false;
        
        for (size_t i = start_pos; i < json_content.length(); ++i) {
            char c = json_content[i];
            
            if (escape) {
                // handle basic escapes
                if (c == 'n') result += '\n';
                else if (c == 't') result += '\t';
                else result += c;
                escape = false;
            } else if (c == '\\') {
                escape = true;
            } else if (c == '"') {
                break; // End of string
            } else {
                result += c;
            }
        }
        return result;
    }

    inline std::string fetch_summary(const std::string& topic) {
        std::string safe_topic = sanitize_topic(topic);
        // Wikipedia API -> JSON with extract
        std::string url = "https://en.wikipedia.org/w/api.php?action=query&format=json&prop=extracts&exintro&explaintext&titles=" + safe_topic;
        
        // Use standard system call to fetch to a tmp file
        std::string tmp_file = "/tmp/brain_research.json";
        std::string command = "curl -s \"" + url + "\" -o " + tmp_file;
        
        int ret = std::system(command.c_str());
        if (ret != 0) return "Connection Failed";

        // Read file
        std::ifstream f(tmp_file);
        if (!f.is_open()) return "Read Failed";
        
        std::stringstream buffer;
        buffer << f.rdbuf();
        
        std::string json = buffer.str();
        std::string text = parse_wikipedia_json(json);
        
        if (text.empty() || text.length() < 10) return "No information found on topic: " + topic;
        return text;
    }
}
