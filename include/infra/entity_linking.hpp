#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace dnn::infra {

// Entity Linking: Connect text mentions to knowledge base entities
class EntityLinking {
public:
    struct Entity {
        std::string id;
        std::string name;
        std::vector<std::string> aliases;
        std::string type;  // PER, ORG, LOC, etc.
        
        Entity() {}
        Entity(const std::string& entity_id, const std::string& entity_name,
              const std::string& entity_type = "")
            : id(entity_id), name(entity_name), type(entity_type) {}
    };
    
    struct Mention {
        std::string text;
        int start_pos;
        int end_pos;
        std::string linked_entity_id;
        float confidence;
        
        Mention() : start_pos(0), end_pos(0), confidence(0.0f) {}
        Mention(const std::string& mention_text, int start, int end)
            : text(mention_text), start_pos(start), end_pos(end), confidence(0.0f) {}
    };
    
    EntityLinking() {}
    
    // Add entity to knowledge base
    void add_entity(const std::string& id, const std::string& name,
                   const std::string& type = "",
                   const std::vector<std::string>& aliases = {}) {
        Entity entity(id, name, type);
        entity.aliases = aliases;
        entities_[id] = entity;
        
        // Index by name and aliases
        name_to_id_[to_lower(name)] = id;
        for (const auto& alias : aliases) {
            name_to_id_[to_lower(alias)] = id;
        }
    }
    
    // Link mentions in text to entities
    std::vector<Mention> link_entities(const std::string& text) {
        std::vector<Mention> mentions;
        
        // Simple exact string matching
        for (const auto& [name, entity_id] : name_to_id_) {
            size_t pos = 0;
            std::string lower_text = to_lower(text);
            
            while ((pos = lower_text.find(name, pos)) != std::string::npos) {
                Mention mention(text.substr(pos, name.length()), pos, pos + name.length());
                mention.linked_entity_id = entity_id;
                mention.confidence = 1.0f;  // Exact match
                mentions.push_back(mention);
                pos += name.length();
            }
        }
        
        // Remove overlapping mentions (keep longer ones)
        remove_overlaps(mentions);
        
        return mentions;
    }
    
    // Link a specific mention
    std::string link_mention(const std::string& mention_text, float min_confidence = 0.5f) {
        std::string lower_mention = to_lower(mention_text);
        
        // Exact match
        auto it = name_to_id_.find(lower_mention);
        if (it != name_to_id_.end()) {
            return it->second;
        }
        
        // Partial match (contains)
        for (const auto& [name, entity_id] : name_to_id_) {
            if (lower_mention.find(name) != std::string::npos ||
                name.find(lower_mention) != std::string::npos) {
                return entity_id;
            }
        }
        
        return "";
    }
    
    // Get entity by ID
    Entity get_entity(const std::string& entity_id) const {
        auto it = entities_.find(entity_id);
        return it != entities_.end() ? it->second : Entity();
    }
    
    // Get all entities
    std::vector<Entity> get_all_entities() const {
        std::vector<Entity> result;
        result.reserve(entities_.size());
        for (const auto& [id, entity] : entities_) {
            result.push_back(entity);
        }
        return result;
    }
    
private:
    std::unordered_map<std::string, Entity> entities_;
    std::unordered_map<std::string, std::string> name_to_id_;
    
    std::string to_lower(std::string s) const {
        std::transform(s.begin(), s.end(), s.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return s;
    }
    
    void remove_overlaps(std::vector<Mention>& mentions) const {
        if (mentions.empty()) return;
        
        // Sort by start position
        std::sort(mentions.begin(), mentions.end(),
                 [](const Mention& a, const Mention& b) {
                     return a.start_pos < b.start_pos;
                 });
        
        std::vector<Mention> filtered;
        filtered.push_back(mentions[0]);
        
        for (size_t i = 1; i < mentions.size(); i++) {
            const Mention& last = filtered.back();
            const Mention& current = mentions[i];
            
            // No overlap
            if (current.start_pos >= last.end_pos) {
                filtered.push_back(current);
            } else {
                // Overlap: keep longer mention
                if (current.end_pos - current.start_pos > last.end_pos - last.start_pos) {
                    filtered.back() = current;
                }
            }
        }
        
        mentions = filtered;
    }
};

} // namespace dnn::infra
