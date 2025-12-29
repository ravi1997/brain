#pragma once
#include <string>
#include <vector>
#include <regex>
#include <unordered_set>

namespace dnn::nlu {

// Open Information Extraction - extract triples without pre-defined relations
class OpenIE {
public:
    struct Extraction {
        std::string arg1;      // Subject
        std::string relation;
        std::string arg2;      // Object
        float confidence;
        
        Extraction() : confidence(0) {}
        Extraction(const std::string& a1, const std::string& rel, 
                  const std::string& a2, float conf = 1.0f)
            : arg1(a1), relation(rel), arg2(a2), confidence(conf) {}
    };
    
    OpenIE() {}
    
    // Extract all relations from text
    std::vector<Extraction> extract(const std::string& text) {
        std::vector<Extraction> extractions;
        
        // Sentence splitting
        auto sentences = split_sentences(text);
        
        for (const auto& sentence : sentences) {
            // Extract from each sentence
            auto sent_extractions = extract_from_sentence(sentence);
            extractions.insert(extractions.end(), 
                             sent_extractions.begin(), sent_extractions.end());
        }
        
        return extractions;
    }
    
private:
    std::vector<std::string> split_sentences(const std::string& text) {
        std::vector<std::string> sentences;
        std::string sentence;
        
        for (char c : text) {
            sentence += c;
            if (c == '.' || c == '!' || c == '?') {
                if (!sentence.empty()) {
                    sentences.push_back(sentence);
                    sentence.clear();
                }
            }
        }
        
        if (!sentence.empty()) {
            sentences.push_back(sentence);
        }
        
        return sentences;
    }
    
    std::vector<Extraction> extract_from_sentence(const std::string& sentence) {
        std::vector<Extraction> extractions;
        
        // Pattern 1: Subject Verb Object
        std::regex svo_pattern(
            R"(([A-Z][a-z]+(?:\s+[A-Z][a-z]+)*)\s+((?:is|are|was|were|has|have|had|does|do|did)\s+\w+|\w+s|\w+ed|\w+ing)\s+([a-z]+(?:\s+[a-z]+)*))"
        );
        
        std::smatch match;
        std::string::const_iterator search_start(sentence.cbegin());
        
        while (std::regex_search(search_start, sentence.cend(), match, svo_pattern)) {
            if (match.size() >= 4) {
                Extraction ext(match[1].str(), match[2].str(), match[3].str(), 0.7f);
                extractions.push_back(ext);
            }
            search_start = match.suffix().first;
        }
        
        // Pattern 2: Noun Phrase, Verb, Prepositional Phrase
        std::regex nvp_pattern(
            R"(([A-Z][a-z]+)\s+(works|lives|studies|teaches)\s+(in|at|for|with)\s+([A-Z][a-z]+))"
        );
        
        search_start = sentence.cbegin();
        while (std::regex_search(search_start, sentence.cend(), match, nvp_pattern)) {
            if (match.size() >= 5) {
                std::string relation = match[2].str() + " " + match[3].str();
                Extraction ext(match[1].str(), relation, match[4].str(), 0.8f);
                extractions.push_back(ext);
            }
            search_start = match.suffix().first;
        }
        
        // Pattern 3: Possession
        std::regex poss_pattern(
            R"(([A-Z][a-z]+)'s\s+(\w+)\s+(?:is|was)\s+([a-z]+(?:\s+[a-z]+)*))"
        );
        
        search_start = sentence.cbegin();
        while (std::regex_search(search_start, sentence.cend(), match, poss_pattern)) {
            if (match.size() >= 4) {
                Extraction ext(match[1].str(), "has " + match[2].str(), match[3].str(), 0.6f);
                extractions.push_back(ext);
            }
            search_start = match.suffix().first;
        }
        
        return extractions;
    }
};

} // namespace dnn::nlu
