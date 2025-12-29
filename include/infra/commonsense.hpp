#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace dnn::infra {

// Common-Sense Reasoning Database
class CommonSenseReasoning {
public:
    struct Fact {
        std::string subject;
        std::string relation;
        std::string object;
        float confidence;
        
        Fact() : confidence(1.0f) {}
        Fact(const std::string& s, const std::string& r, const std::string& o, float c = 1.0f)
            : subject(s), relation(r), object(o), confidence(c) {}
    };
    
    CommonSenseReasoning() {
        initialize_common_sense_kb();
    }
    
    // Query common sense knowledge
    std::vector<Fact> query(const std::string& subject, const std::string& relation) {
        std::vector<Fact> results;
        
        for (const auto& fact : knowledge_base_) {
            if ((subject.empty() || fact.subject == subject) &&
                (relation.empty() || fact.relation == relation)) {
                results.push_back(fact);
            }
        }
        
        return results;
    }
    
    // Add new fact
    void add_fact(const Fact& fact) {
        knowledge_base_.push_back(fact);
    }
    
    // Check if something is plausible
    bool is_plausible(const std::string& subject, const std::string& relation,
                     const std::string& object) {
        // Check if fact exists
        for (const auto& fact : knowledge_base_) {
            if (fact.subject == subject && fact.relation == relation &&
                fact.object == object) {
                return fact.confidence > 0.5f;
            }
        }
        
        // Check for contradictions
        for (const auto& fact : knowledge_base_) {
            if (fact.subject == subject && fact.relation == relation &&
                fact.object != object && fact.confidence > 0.8f) {
                return false;  // Contradicts strong belief
            }
        }
        
        return true;  // No information, assume possible
    }
    
    // Infer missing facts using analogies
    std::vector<Fact> infer_by_analogy(const std::string& subject) {
        std::vector<Fact> inferred;
        
        // Find similar entities
        auto similar = find_similar_entities(subject);
        
        for (const auto& sim_entity : similar) {
            // Transfer properties
            auto facts = query(sim_entity, "");
            
            for (const auto& fact : facts) {
                // Create analogous fact
                Fact new_fact(subject, fact.relation, fact.object, 
                             fact.confidence * 0.6f);  // Reduce confidence
                
                if (is_plausible(new_fact.subject, new_fact.relation, new_fact.object)) {
                    inferred.push_back(new_fact);
                }
            }
        }
        
        return inferred;
    }
    
    // Answer "why" questions
    std::string explain(const std::string& subject, const std::string& relation,
                       const std::string& object) {
        // Find supporting facts
        for (const auto& fact : knowledge_base_) {
            if (fact.subject == subject && fact.relation == relation &&
                fact.object == object) {
                return "Because " + subject + " typically " + relation + " " + object;
            }
        }
        
        // Try to construct explanation from related facts
        auto related = query(subject, "");
        
        if (!related.empty()) {
            std::string exp = "Because " + subject + " is a kind of ";
            
            for (const auto& fact : related) {
                if (fact.relation == "IsA") {
                    exp += fact.object + ", which ";
                    
                    auto type_facts = query(fact.object, relation);
                    if (!type_facts.empty()) {
                        exp += relation + " " + object;
                        return exp;
                    }
                }
            }
        }
        
        return "No explanation found";
    }
    
private:
    std::vector<Fact> knowledge_base_;
    
    void initialize_common_sense_kb() {
        // Physical properties
        add_fact({"water", "IsA", "liquid", 1.0f});
        add_fact({"ice", "IsA", "solid", 1.0f});
        add_fact({"water", "freezes_at", "0_celsius", 1.0f});
        add_fact({"water", "boils_at", "100_celsius", 1.0f});
        
        // Animals
        add_fact({"dog", "IsA", "animal", 1.0f});
        add_fact({"cat", "IsA", "animal", 1.0f});
        add_fact({"bird", "IsA", "animal", 1.0f});
        add_fact({"dog", "has", "four_legs", 0.95f});
        add_fact({"cat", "has", "four_legs", 0.95f});
        add_fact({"bird", "has", "wings", 0.9f});
        add_fact({"bird", "can", "fly", 0.85f});
        add_fact({"dog", "can", "bark", 0.95f});
        add_fact({"cat", "can", "meow", 0.95f});
        
        // People
        add_fact({"person", "has", "two_legs", 0.99f});
        add_fact({"person", "can", "think", 1.0f});
        add_fact({"person", "can", "speak", 0.99f});
        add_fact({"person", "needs", "food", 1.0f});
        add_fact({"person", "needs", "water", 1.0f});
        add_fact({"person", "needs", "sleep", 1.0f});
        
        // Objects
        add_fact({"car", "IsA", "vehicle", 1.0f});
        add_fact({"bicycle", "IsA", "vehicle", 1.0f});
        add_fact({"car", "has", "wheels", 1.0f});
        add_fact({"car", "has", "engine", 0.99f});
        add_fact({"car", "uses", "fuel", 0.9f});
        
        // Locations
        add_fact({"house", "IsA", "building", 1.0f});
        add_fact({"school", "IsA", "building", 1.0f});
        add_fact({"house", "has", "rooms", 0.99f});
        add_fact({"house", "has", "door", 1.0f});
        
        // Actions
        add_fact({"eating", "requires", "food", 1.0f});
        add_fact({"driving", "requires", "vehicle", 1.0f});
        add_fact({"sleeping", "requires", "bed", 0.8f});
        add_fact({"writing", "requires", "pen", 0.7f});
        
        // Temporal
        add_fact({"day", "follows", "night", 1.0f});
        add_fact({"summer", "is_warmer_than", "winter", 0.9f});
        add_fact({"morning", "comes_before", "afternoon", 1.0f});
        
        // Causality
        add_fact({"rain", "causes", "wetness", 0.95f});
        add_fact({"fire", "causes", "heat", 1.0f});
        add_fact({"cutting", "causes", "separation", 1.0f});
        
        // Spatial
        add_fact({"ceiling", "is_above", "floor", 1.0f});
        add_fact({"sky", "is_above", "ground", 1.0f});
        add_fact({"inside", "opposite_of", "outside", 1.0f});
    }
    
    std::vector<std::string> find_similar_entities(const std::string& entity) {
        std::vector<std::string> similar;
        
        // Find entities with same type
        std::string entity_type;
        
        for (const auto& fact : knowledge_base_) {
            if (fact.subject == entity && fact.relation == "IsA") {
                entity_type = fact.object;
                break;
            }
        }
        
        if (!entity_type.empty()) {
            for (const auto& fact : knowledge_base_) {
                if (fact.relation == "IsA" && fact.object == entity_type &&
                    fact.subject != entity) {
                    similar.push_back(fact.subject);
                }
            }
        }
        
        return similar;
    }
};

} // namespace dnn::infra
