#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace dnn::perception {

// Scene Graph Generation - extract objects and relationships
class SceneGraphGenerator {
public:
    struct Object {
        std::string name;
        int id;
        float confidence;
        std::vector<float> bbox;  // x, y, w, h
        
        Object() : id(0), confidence(0.0f) {}
        Object(const std::string& n, int i, float c) 
            : name(n), id(i), confidence(c) {}
    };
    
    struct Relationship {
        int subject_id;
        std::string predicate;
        int object_id;
        float confidence;
        
        Relationship() : subject_id(0), object_id(0), confidence(0.0f) {}
        Relationship(int subj, const std::string& pred, int obj, float conf)
            : subject_id(subj), predicate(pred), object_id(obj), confidence(conf) {}
    };
    
    struct SceneGraph {
        std::vector<Object> objects;
        std::vector<Relationship> relationships;
    };
    
    SceneGraphGenerator() {}
    
    // Generate scene graph from detected objects
    SceneGraph generate(const std::vector<Object>& detected_objects) {
        SceneGraph graph;
        graph.objects = detected_objects;
        
        // Infer relationships based on spatial arrangement
        for (size_t i = 0; i < detected_objects.size(); i++) {
            for (size_t j = i + 1; j < detected_objects.size(); j++) {
                auto rel = infer_relationship(detected_objects[i], detected_objects[j]);
                if (rel.confidence > 0.3f) {
                    graph.relationships.push_back(rel);
                }
            }
        }
        
        return graph;
    }
    
    // Query scene graph
    std::vector<Object> query_objects(const SceneGraph& graph, const std::string& name) {
        std::vector<Object> results;
        for (const auto& obj : graph.objects) {
            if (obj.name == name) {
                results.push_back(obj);
            }
        }
        return results;
    }
    
    // Query relationships
    std::vector<Relationship> query_relationships(const SceneGraph& graph, 
                                                  const std::string& predicate) {
        std::vector<Relationship> results;
        for (const auto& rel : graph.relationships) {
            if (rel.predicate == predicate) {
                results.push_back(rel);
            }
        }
        return results;
    }
    
private:
    Relationship infer_relationship(const Object& obj1, const Object& obj2) {
        Relationship rel;
        rel.subject_id = obj1.id;
        rel.object_id = obj2.id;
        rel.confidence = 0.5f;
        
        // Simple spatial relationship inference
        if (obj1.bbox.size() >= 4 && obj2.bbox.size() >= 4) {
            float obj1_y = obj1.bbox[1];
            float obj2_y = obj2.bbox[1];
            float obj1_x = obj1.bbox[0];
            float obj2_x = obj2.bbox[0];
            
            // Above/below
            if (obj1_y < obj2_y - 50) {
                rel.predicate = "above";
                rel.confidence = 0.8f;
            } else if (obj1_y > obj2_y + 50) {
                rel.predicate = "below";
                rel.confidence = 0.8f;
            }
            // Left/right
            else if (obj1_x < obj2_x - 50) {
                rel.predicate = "left_of";
                rel.confidence = 0.7f;
            } else if (obj1_x > obj2_x + 50) {
                rel.predicate = "right_of";
                rel.confidence = 0.7f;
            } else {
                rel.predicate = "near";
                rel.confidence = 0.6f;
            }
        }
        
        return rel;
    }
};

} // namespace dnn::perception
