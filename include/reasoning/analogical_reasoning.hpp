#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace dnn::reasoning {

// Analogical Reasoning - structure mapping
class AnalogicalReasoning {
public:
    struct Relation {
        std::string type;
        std::string source;
        std::string target;
        
        Relation() {}
        Relation(const std::string& t, const std::string& s, const std::string& tgt)
            : type(t), source(s), target(tgt) {}
    };
    
    struct Analogy {
        std::vector<Relation> source_domain;
        std::vector<Relation> target_domain;
        std::vector<std::pair<std::string, std::string>> mappings;
        float similarity_score;
        
        Analogy() : similarity_score(0.0f) {}
    };
    
    AnalogicalReasoning() {}
    
    // Find analogical mapping between source and target
    Analogy find_analogy(const std::vector<Relation>& source,
                        const std::vector<Relation>& target) {
        Analogy analogy;
        analogy.source_domain = source;
        analogy.target_domain = target;
        
        // Find structural similarities
        for (const auto& s_rel : source) {
            for (const auto& t_rel : target) {
                if (s_rel.type == t_rel.type) {
                    // Same relation type - potential mapping
                    analogy.mappings.emplace_back(s_rel.source, t_rel.source);
                    analogy.mappings.emplace_back(s_rel.target, t_rel.target);
                    analogy.similarity_score += 1.0f;
                }
            }
        }
        
        // Remove duplicate mappings
        std::sort(analogy.mappings.begin(), analogy.mappings.end());
        analogy.mappings.erase(
            std::unique(analogy.mappings.begin(), analogy.mappings.end()),
            analogy.mappings.end()
        );
        
        // Normalize score
        if (!source.empty()) {
            analogy.similarity_score /= source.size();
        }
        
        return analogy;
    }
    
    // Transfer knowledge from source to target
    std::vector<Relation> transfer_knowledge(const Analogy& analogy,
                                            const std::vector<Relation>& source_knowledge) {
        std::vector<Relation> transferred;
        
        // Build mapping dictionary
        std::unordered_map<std::string, std::string> map_dict;
        for (const auto& [src, tgt] : analogy.mappings) {
            map_dict[src] = tgt;
        }
        
        // Transfer each relation
        for (const auto& rel : source_knowledge) {
            Relation new_rel = rel;
            
            // Map source and target if they exist in mapping
            if (map_dict.count(rel.source)) {
                new_rel.source = map_dict[rel.source];
            }
            if (map_dict.count(rel.target)) {
                new_rel.target = map_dict[rel.target];
            }
            
            transferred.push_back(new_rel);
        }
        
        return transferred;
    }
    
    // Evaluate analogy quality
    float evaluate(const Analogy& analogy) const {
        return analogy.similarity_score;
    }
    
};

} // namespace dnn::reasoning
