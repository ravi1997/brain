#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace dnn::infra {

// Semantic Web Reasoning with OWL/RDF concepts
class SemanticWebReasoning {
public:
    struct Triple {
        std::string subject;
        std::string predicate;
        std::string object;
        
        Triple() {}
        Triple(const std::string& s, const std::string& p, const std::string& o)
            : subject(s), predicate(p), object(o) {}
        
        std::string to_string() const {
            return subject + " " + predicate + " " + object;
        }
    };
    
    struct OntologyClass {
        std::string name;
        std::vector<std::string> superclasses;
        std::vector<std::string> properties;
        
        OntologyClass() {}
        OntologyClass(const std::string& n) : name(n) {}
    };
    
    SemanticWebReasoning() {
        // Add built-in RDF/RDFS predicates
        add_predicate_property("rdf:type", "defines class membership");
        add_predicate_property("rdfs:subClassOf", "class hierarchy");
        add_predicate_property("rdfs:subPropertyOf", "property hierarchy");
        add_predicate_property("owl:sameAs", "identity");
        add_predicate_property("owl:inverseOf", "inverse properties");
    }
    
    // Add RDF triple
    void add_triple(const Triple& triple) {
        triples_.push_back(triple);
        index_triple(triple);
    }
    
    // Add ontology class
    void add_class(const OntologyClass& cls) {
        classes_[cls.name] = cls;
    }
    
    // SPARQL-like query: find all triples matching pattern
    std::vector<Triple> query(const std::string& subject_pattern,
                             const std::string& predicate_pattern,
                             const std::string& object_pattern) {
        std::vector<Triple> results;
        
        for (const auto& triple : triples_) {
            if (matches(triple.subject, subject_pattern) &&
                matches(triple.predicate, predicate_pattern) &&
                matches(triple.object, object_pattern)) {
                results.push_back(triple);
            }
        }
        
        return results;
    }
    
    // Infer new triples using RDFS reasoning
    std::vector<Triple> infer_rdfs() {
        std::vector<Triple> inferred;
        
        // Rule: rdfs:subClassOf is transitive
        for (const auto& t1 : triples_) {
            if (t1.predicate == "rdfs:subClassOf") {
                for (const auto& t2 : triples_) {
                    if (t2.predicate == "rdfs:subClassOf" &&
                        t2.subject == t1.object) {
                        // A subClassOf B, B subClassOf C => A subClassOf C
                        Triple inferred_triple(t1.subject, "rdfs:subClassOf", t2.object);
                        
                        if (!contains_triple(inferred_triple) &&
                            !contains_triple_in_vec(inferred, inferred_triple)) {
                            inferred.push_back(inferred_triple);
                        }
                    }
                }
            }
        }
        
        // Rule: type propagation through subClassOf
        for (const auto& t1 : triples_) {
            if (t1.predicate == "rdf:type") {
                for (const auto& t2 : triples_) {
                    if (t2.predicate == "rdfs:subClassOf" &&
                        t2.subject == t1.object) {
                        // x type A, A subClassOf B => x type B
                        Triple inferred_triple(t1.subject, "rdf:type", t2.object);
                        
                        if (!contains_triple(inferred_triple) &&
                            !contains_triple_in_vec(inferred, inferred_triple)) {
                            inferred.push_back(inferred_triple);
                        }
                    }
                }
            }
        }
        
        return inferred;
    }
    
    // OWL reasoning: property characteristics
    std::vector<Triple> infer_owl() {
        std::vector<Triple> inferred;
        
        // Symmetric property
        for (const auto& [prop, chars] : property_characteristics_) {
            if (chars.count("symmetric")) {
                for (const auto& triple : triples_) {
                    if (triple.predicate == prop) {
                        // P(x,y) and P is symmetric => P(y,x)
                        Triple symmetric(triple.object, prop, triple.subject);
                        
                        if (!contains_triple(symmetric) &&
                            !contains_triple_in_vec(inferred, symmetric)) {
                            inferred.push_back(symmetric);
                        }
                    }
                }
            }
            
            // Transitive property
            if (chars.count("transitive")) {
                for (const auto& t1 : triples_) {
                    if (t1.predicate == prop) {
                        for (const auto& t2 : triples_) {
                            if (t2.predicate == prop && t2.subject == t1.object) {
                                // P(x,y) and P(y,z) and P is transitive => P(x,z)
                                Triple transitive(t1.subject, prop, t2.object);
                                
                                if (!contains_triple(transitive) &&
                                    !contains_triple_in_vec(inferred, transitive)) {
                                    inferred.push_back(transitive);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // owl:sameAs reasoning
        for (const auto& triple : triples_) {
            if (triple.predicate == "owl:sameAs") {
                // Replace subject in all triples
                for (const auto& t : triples_) {
                    if (t.subject == triple.subject) {
                        Triple replaced(triple.object, t.predicate, t.object);
                        
                        if (!contains_triple(replaced) &&
                            !contains_triple_in_vec(inferred, replaced)) {
                            inferred.push_back(replaced);
                        }
                    }
                }
            }
        }
        
        return inferred;
    }
    
    // Add property characteristic
    void add_property_characteristic(const std::string& property,
                                     const std::string& characteristic) {
        property_characteristics_[property].insert(characteristic);
    }
    
    // Check if instance belongs to class (with inference)
    bool is_instance_of(const std::string& instance, const std::string& cls) {
        // Direct check
        for (const auto& triple : triples_) {
            if (triple.subject == instance &&
                triple.predicate == "rdf:type" &&
                triple.object == cls) {
                return true;
            }
        }
        
        // Check through subclass hierarchy
        auto inferred = infer_rdfs();
        for (const auto& triple : inferred) {
            if (triple.subject == instance &&
                triple.predicate == "rdf:type" &&
                triple.object == cls) {
                return true;
            }
        }
        
        return false;
    }
    
private:
    std::vector<Triple> triples_;
    std::unordered_map<std::string, OntologyClass> classes_;
    std::unordered_map<std::string, std::string> predicate_descriptions_;
    std::unordered_map<std::string, std::unordered_set<std::string>> property_characteristics_;
    
    // Index for faster queries
    std::unordered_map<std::string, std::vector<Triple>> subject_index_;
    std::unordered_map<std::string, std::vector<Triple>> predicate_index_;
    std::unordered_map<std::string, std::vector<Triple>> object_index_;
    
    void add_predicate_property(const std::string& pred, const std::string& desc) {
        predicate_descriptions_[pred] = desc;
    }
    
    void index_triple(const Triple& triple) {
        subject_index_[triple.subject].push_back(triple);
        predicate_index_[triple.predicate].push_back(triple);
        object_index_[triple.object].push_back(triple);
    }
    
    bool matches(const std::string& value, const std::string& pattern) {
        if (pattern == "?" || pattern == "*" || pattern.empty()) {
            return true;  // Wildcard
        }
        return value == pattern;
    }
    
    bool contains_triple(const Triple& triple) {
        for (const auto& t : triples_) {
            if (t.subject == triple.subject &&
                t.predicate == triple.predicate &&
                t.object == triple.object) {
                return true;
            }
        }
        return false;
    }
    
    bool contains_triple_in_vec(const std::vector<Triple>& vec, const Triple& triple) {
        for (const auto& t : vec) {
            if (t.subject == triple.subject &&
                t.predicate == triple.predicate &&
                t.object == triple.object) {
                return true;
            }
        }
        return false;
    }
};

} // namespace dnn::infra
