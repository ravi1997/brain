#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace dnn::infra {

// Knowledge Base Completion using rule-based inference
class KnowledgeBaseCompletion {
public:
    struct Fact {
        std::string subject;
        std::string relation;
        std::string object;
        float confidence;
        
        Fact() : confidence(1.0f) {}
        Fact(const std::string& s, const std::string& r, const std::string& o, float c = 1.0f)
            : subject(s), relation(r), object(o), confidence(c) {}
        
        std::string to_string() const {
            return subject + "|" + relation + "|" + object;
        }
    };
    
    KnowledgeBaseCompletion() {
        initialize_rules();
    }
    
    // Add fact to knowledge base
    void add_fact(const Fact& fact) {
        kb_[fact.to_string()] = fact;
    }
    
    // Complete knowledge base by inferring new facts
    std::vector<Fact> complete() {
        std::vector<Fact> inferred;
        bool changed = true;
        int max_iterations = 10;
        int iteration = 0;
        
        while (changed && iteration < max_iterations) {
            changed = false;
            iteration++;
            
            // Apply inference rules
            for (const auto& rule : rules_) {
                auto new_facts = apply_rule(rule);
                
                for (const auto& fact : new_facts) {
                    if (kb_.find(fact.to_string()) == kb_.end()) {
                        kb_[fact.to_string()] = fact;
                        inferred.push_back(fact);
                        changed = true;
                    }
                }
            }
        }
        
        return inferred;
    }
    
    // Query knowledge base
    std::vector<Fact> query(const std::string& subject = "", 
                           const std::string& relation = "",
                           const std::string& object = "") {
        std::vector<Fact> results;
        
        for (const auto& [key, fact] : kb_) {
            bool match = true;
            
            if (!subject.empty() && fact.subject != subject) match = false;
            if (!relation.empty() && fact.relation != relation) match = false;
            if (!object.empty() && fact.object != object) match = false;
            
            if (match) {
                results.push_back(fact);
            }
        }
        
        return results;
    }
    
private:
    struct InferenceRule {
        std::string premise1_rel;
        std::string premise2_rel;
        std::string conclusion_rel;
        bool transitive;  // If true, chain subjects/objects
        
        InferenceRule(const std::string& p1, const std::string& p2, 
                     const std::string& c, bool trans = false)
            : premise1_rel(p1), premise2_rel(p2), conclusion_rel(c), transitive(trans) {}
    };
    
    std::unordered_map<std::string, Fact> kb_;
    std::vector<InferenceRule> rules_;
    
    void initialize_rules() {
        // Transitive rules
        rules_.emplace_back("subclass_of", "subclass_of", "subclass_of", true);
        rules_.emplace_back("part_of", "part_of", "part_of", true);
        rules_.emplace_back("located_in", "located_in", "located_in", true);
        
        // Symmetric rules
        rules_.emplace_back("spouse", "", "spouse", false);  // If A spouse B, then B spouse A
        
        // Inverse rules
        rules_.emplace_back("parent_of", "", "child_of", false);
    }
    
    std::vector<Fact> apply_rule(const InferenceRule& rule) {
        std::vector<Fact> inferred;
        
        if (rule.transitive) {
            // Transitivity: If (A,R,B) and (B,R,C) then (A,R,C)
            auto facts1 = query("", rule.premise1_rel, "");
            auto facts2 = query("", rule.premise2_rel, "");
            
            for (const auto& f1 : facts1) {
                for (const auto& f2 : facts2) {
                    if (f1.object == f2.subject) {
                        Fact new_fact(f1.subject, rule.conclusion_rel, f2.object,
                                     std::min(f1.confidence, f2.confidence) * 0.9f);
                        inferred.push_back(new_fact);
                    }
                }
            }
        } else if (rule.premise2_rel.empty()) {
            // Symmetric or inverse
            auto facts = query("", rule.premise1_rel, "");
            
            for (const auto& f : facts) {
                Fact new_fact(f.object, rule.conclusion_rel, f.subject, f.confidence * 0.95f);
                inferred.push_back(new_fact);
            }
        }
        
        return inferred;
    }
};

} // namespace dnn::infra
