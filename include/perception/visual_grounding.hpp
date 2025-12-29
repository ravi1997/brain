#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace dnn::perception {

// Visual Grounding - linking language phrases to image regions
class VisualGrounding {
public:
    struct Region {
        float x, y, w, h;  // Bounding box
        std::vector<float> visual_features;
        std::string description;
        float confidence;
        
        Region() : x(0), y(0), w(0), h(0), confidence(0) {}
        Region(float x_, float y_, float w_, float h_)
            : x(x_), y(y_), w(w_), h(h_), confidence(0) {}
    };
    
    struct Phrase {
        std::string text;
        std::vector<std::string> keywords;
        std::string phrase_type;  // "object", "attribute", "relation"
        
        Phrase() {}
        Phrase(const std::string& t) : text(t) {
            keywords = extract_keywords(t);
            phrase_type = classify_phrase(t);
        }
        
    private:
        std::vector<std::string> extract_keywords(const std::string& text) const {
            std::vector<std::string> keywords;
            std::string word;
            
            for (char c : text) {
                if (std::isalnum(c)) {
                    word += std::tolower(c);
                } else if (!word.empty()) {
                    if (word != "the" && word != "a" && word != "an") {
                        keywords.push_back(word);
                    }
                    word.clear();
                }
            }
            
            if (!word.empty()) {
                keywords.push_back(word);
            }
            
            return keywords;
        }
        
        std::string classify_phrase(const std::string& text) const {
            std::string lower = text;
            for (char& c : lower) c = std::tolower(c);
            
            // Check for attributes (colors, sizes, etc.)
            std::vector<std::string> attributes = {
                "red", "blue", "green", "yellow", "black", "white",
                "big", "small", "large", "tiny", "tall", "short"
            };
            
            for (const auto& attr : attributes) {
                if (lower.find(attr) != std::string::npos) {
                    return "attribute";
                }
            }
            
            // Check for spatial relations
            std::vector<std::string> relations = {
                "left", "right", "above", "below", "near", "on", "in", "under"
            };
            
            for (const auto& rel : relations) {
                if (lower.find(rel) != std::string::npos) {
                    return "relation";
                }
            }
            
            return "object";
        }
    };
    
    VisualGrounding() {
        initialize_vocabulary();
    }
    
    // Ground phrase to image region
    Region ground(const Phrase& phrase, const std::vector<Region>& regions) {
        if (regions.empty()) {
            return Region();
        }
        
        // Score each region based on phrase
        std::vector<float> scores;
        
        for (const auto& region : regions) {
            float score = compute_matching_score(phrase, region);
            scores.push_back(score);
        }
        
        // Find best match
        int best_idx = std::max_element(scores.begin(), scores.end()) - scores.begin();
        
        Region grounded = regions[best_idx];
        grounded.confidence = scores[best_idx];
        
        return grounded;
    }
    
    // Ground multiple phrases
    std::vector<Region> ground_multiple(const std::vector<Phrase>& phrases,
                                       const std::vector<Region>& regions) {
        std::vector<Region> grounded_regions;
        
        for (const auto& phrase : phrases) {
            grounded_regions.push_back(ground(phrase, regions));
        }
        
        return grounded_regions;
    }
    
    // Referring expression comprehension
    Region comprehend_expression(const std::string& expression,
                                const std::vector<Region>& regions) {
        Phrase phrase(expression);
        return ground(phrase, regions);
    }
    
    // Spatial relationship grounding
    Region ground_spatial_relation(const std::string& relation,
                                  const Region& reference,
                                  const std::vector<Region>& candidates) {
        std::vector<float> scores;
        
        for (const auto& candidate : candidates) {
            float score = compute_spatial_score(relation, reference, candidate);
            scores.push_back(score);
        }
        
        if (scores.empty()) {
            return Region();
        }
        
        int best_idx = std::max_element(scores.begin(), scores.end()) - scores.begin();
        
        Region grounded = candidates[best_idx];
        grounded.confidence = scores[best_idx];
        
        return grounded;
    }
    
private:
    std::unordered_map<std::string, std::vector<std::string>> object_vocab_;
    std::unordered_map<std::string, std::vector<std::string>> attribute_vocab_;
    
    void initialize_vocabulary() {
        // Object vocabulary
        object_vocab_["person"] = {"person", "man", "woman", "child", "people"};
        object_vocab_["car"] = {"car", "vehicle", "automobile"};
        object_vocab_["dog"] = {"dog", "puppy", "canine"};
        object_vocab_["cat"] = {"cat", "kitten", "feline"};
        object_vocab_["tree"] = {"tree", "plant"};
        
        // Attribute vocabulary
        attribute_vocab_["red"] = {"red", "crimson", "scarlet"};
        attribute_vocab_["blue"] = {"blue", "azure"};
        attribute_vocab_["large"] = {"large", "big", "huge", "enormous"};
        attribute_vocab_["small"] = {"small", "tiny", "little"};
    }
    
    float compute_matching_score(const Phrase& phrase, const Region& region) {
        float score = 0.0f;
        
        // Match keywords with region description
        for (const auto& keyword : phrase.keywords) {
            if (region.description.find(keyword) != std::string::npos) {
                score += 1.0f;
            }
            
            // Check vocabulary
            for (const auto& [obj, synonyms] : object_vocab_) {
                if (region.description.find(obj) != std::string::npos) {
                    for (const auto& syn : synonyms) {
                        if (keyword == syn) {
                            score += 0.8f;
                        }
                    }
                }
            }
        }
        
        // Normalize by number of keywords
        if (!phrase.keywords.empty()) {
            score /= phrase.keywords.size();
        }
        
        return score;
    }
    
    float compute_spatial_score(const std::string& relation,
                               const Region& reference,
                               const Region& candidate) {
       // Compute relative position
        float ref_cx = reference.x + reference.w / 2;
        float ref_cy = reference.y + reference.h / 2;
        float cand_cx = candidate.x + candidate.w / 2;
        float cand_cy = candidate.y + candidate.h / 2;
        
        float dx = cand_cx - ref_cx;
        float dy = cand_cy - ref_cy;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        std::string rel_lower = relation;
        for (char& c : rel_lower) c = std::tolower(c);
        
        // Score based on relation
        if (rel_lower.find("left") != std::string::npos) {
            return dx < 0 ? (1.0f / (1.0f + dist)) : 0.0f;
        } else if (rel_lower.find("right") != std::string::npos) {
            return dx > 0 ? (1.0f / (1.0f + dist)) : 0.0f;
        } else if (rel_lower.find("above") != std::string::npos) {
            return dy < 0 ? (1.0f / (1.0f + dist)) : 0.0f;
        } else if (rel_lower.find("below") != std::string::npos) {
            return dy > 0 ? (1.0f / (1.0f + dist)) : 0.0f;
        } else if (rel_lower.find("near") != std::string::npos) {
            return 1.0f / (1.0f + dist);
        }
        
        return 0.0f;
    }
};

} // namespace dnn::perception
