#pragma once
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>

namespace dnn::nlu {

// Relation Extraction from text
class RelationExtraction {
public:
    struct Triple {
        std::string subject;
        std::string relation;
        std::string object;
        float confidence;
        
        Triple() : confidence(0.0f) {}
        Triple(const std::string& s, const std::string& r, const std::string& o, float c = 1.0f)
            : subject(s), relation(r), object(o), confidence(c) {}
    };
    
    RelationExtraction() {
        // Define pattern-based extraction rules
        patterns_ = {
            {"is a", R"((\w+)\s+is\s+a\s+(\w+))"},
            {"located in", R"((\w+)\s+(?:is\s+)?located\s+in\s+(\w+))"},
            {"works for", R"((\w+)\s+works\s+for\s+(\w+))"},
            {"born in", R"((\w+)\s+(?:was\s+)?born\s+in\s+(\w+))"},
            {"married to", R"((\w+)\s+(?:is\s+)?married\s+to\s+(\w+))"},
            {"capital of", R"((\w+)\s+(?:is\s+)?(?:the\s+)?capital\s+of\s+(\w+))"},
            {"part of", R"((\w+)\s+(?:is\s+)?(?:a\s+)?part\s+of\s+(\w+))"},
            {"created by", R"((\w+)\s+(?:was\s+)?created\s+by\s+(\w+))"}
        };
    }
    
    // Extract relations from text
    std::vector<Triple> extract(const std::string& text) {
        std::vector<Triple> triples;
        
        for (const auto& [relation, pattern] : patterns_) {
            std::regex re(pattern, std::regex::icase);
            std::smatch match;
            
            std::string::const_iterator search_start(text.cbegin());
            while (std::regex_search(search_start, text.cend(), match, re)) {
                if (match.size() >= 3) {
                    Triple triple(match[1].str(), relation, match[2].str(), 0.8f);
                    triples.push_back(triple);
                }
                search_start = match.suffix().first;
            }
        }
        
        // Dependency-based extraction (simplified)
        auto dep_triples = extract_from_dependencies(text);
        triples.insert(triples.end(), dep_triples.begin(), dep_triples.end());
        
        return triples;
    }
    
private:
    std::unordered_map<std::string, std::string> patterns_;
    
    std::vector<Triple> extract_from_dependencies(const std::string& text) {
        std::vector<Triple> triples;
        
        // Simple verb-based extraction: Subject Verb Object
        std::regex svo_pattern(R"((\w+)\s+(owns|likes|has|loves|hates)\s+(\w+))");
        std::smatch match;
        
        std::string::const_iterator search_start(text.cbegin());
        while (std::regex_search(search_start, text.cend(), match, svo_pattern)) {
            if (match.size() >= 4) {
                Triple triple(match[1].str(), match[2].str(), match[3].str(), 0.7f);
                triples.push_back(triple);
            }
            search_start = match.suffix().first;
        }
        
        return triples;
    }
};

} // namespace dnn::nlu
