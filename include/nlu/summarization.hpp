#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace dnn::nlu {

// Text summarization
class Summarization {
public:
    Summarization() {}
    
    // Extractive summarization (select important sentences)
    std::string summarize(const std::string& text, int num_sentences = 3) {
        auto sentences = split_sentences(text);
        
        if (sentences.size() <= num_sentences) {
            return text;
        }
        
        // Score sentences
        std::vector<std::pair<int, float>> scored_sentences;
        for (size_t i = 0; i < sentences.size(); i++) {
            float score = sentence_importance(sentences[i], text);
            scored_sentences.emplace_back(i, score);
        }
        
        // Sort by score
        std::sort(scored_sentences.begin(), scored_sentences.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // Select top sentences and reorder by position
        std::vector<int> selected_indices;
        for (int i = 0; i < num_sentences && i < static_cast<int>(scored_sentences.size()); i++) {
            selected_indices.push_back(scored_sentences[i].first);
        }
        std::sort(selected_indices.begin(), selected_indices.end());
        
        // Build summary
        std::string summary;
        for (int idx : selected_indices) {
            if (!summary.empty()) summary += " ";
            summary += sentences[idx];
        }
        
        return summary;
    }
    
private:
    std::vector<std::string> split_sentences(const std::string& text) {
        std::vector<std::string> sentences;
        std::string sentence;
        
        for (size_t i = 0; i < text.size(); i++) {
            sentence += text[i];
            
            if ((text[i] == '.' || text[i] == '!' || text[i] == '?') &&
                (i + 1 >= text.size() || text[i + 1] == ' ')) {
                sentences.push_back(sentence);
                sentence.clear();
            }
        }
        
        if (!sentence.empty()) {
            sentences.push_back(sentence);
        }
        
        return sentences;
    }
    
    float sentence_importance(const std::string& sentence, const std::string& full_text) {
        float score = 0.0f;
        
        // Position score (first sentences are more important)
        size_t pos = full_text.find(sentence);
        if (pos < full_text.size() / 3) {
            score += 0.5f;
        }
        
        // Length score (prefer moderate length)
        int word_count = count_words(sentence);
        if (word_count >= 10 && word_count <= 30) {
            score += 0.3f;
        }
        
        // Keyword presence
        std::string lower = to_lower(sentence);
        if (lower.find("important") != std::string::npos ||
            lower.find("significant") != std::string::npos ||
            lower.find("key") != std::string::npos ||
            lower.find("main") != std::string::npos) {
            score += 0.2f;
        }
        
        return score;
    }
    
    int count_words(const std::string& text) {
        int count = 0;
        bool in_word = false;
        
        for (char c : text) {
            if (c == ' ' || c == '\n' || c == '\t') {
                in_word = false;
            } else if (!in_word) {
                count++;
                in_word = true;
            }
        }
        
        return count;
    }
    
    std::string to_lower(std::string s) {
        for (char& c : s) c = std::tolower(c);
        return s;
    }
};

} // namespace dnn::nlu
