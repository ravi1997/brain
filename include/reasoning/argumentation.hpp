#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace dnn::reasoning {

// Argumentation Framework for defeasible reasoning
class ArgumentationFramework {
public:
    struct Argument {
        std::string id;
        std::string claim;
        std::vector<std::string> premises;
        float strength;
        
        Argument() : strength(1.0f) {}
        Argument(const std::string& i, const std::string& c, float s = 1.0f)
            : id(i), claim(c), strength(s) {}
    };
    
    struct Attack {
        std::string attacker_id;
        std::string target_id;
        float strength;
        
        Attack() : strength(1.0f) {}
        Attack(const std::string& from, const std::string& to, float s = 1.0f)
            : attacker_id(from), target_id(to), strength(s) {}
    };
    
    enum class Semantics {
        GROUNDED,     // Single minimal extension
        PREFERRED,    // Maximal admissible sets
        STABLE        // Attack all outside arguments
    };
    
    ArgumentationFramework() {}
    
    // Add argument
    void add_argument(const Argument& arg) {
        arguments_[arg.id] = arg;
    }
    
    // Add attack relation
    void add_attack(const Attack& attack) {
        attacks_.push_back(attack);
    }
    
    // Compute extensions under given semantics
    std::vector<std::unordered_set<std::string>> 
    compute_extensions(Semantics semantics) {
        switch (semantics) {
            case Semantics::GROUNDED:
                return {compute_grounded_extension()};
            case Semantics::PREFERRED:
                return compute_preferred_extensions();
            case Semantics::STABLE:
                return compute_stable_extensions();
            default:
                return {};
        }
    }
    
    // Check if argument is acceptable
    bool is_acceptable(const std::string& arg_id,
                      const std::unordered_set<std::string>& set) {
        // All attackers of arg must be attacked by set
        for (const auto& attack : attacks_) {
            if (attack.target_id == arg_id) {
                // Is the attacker defended against?
                if (!is_attacked_by(attack.attacker_id, set)) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    // Check if set is conflict-free
    bool is_conflict_free(const std::unordered_set<std::string>& set) {
        for (const auto& arg1 : set) {
            for (const auto& arg2 : set) {
                if (attacks_relation(arg1, arg2)) {
                    return false;
                }
            }
        }
        return true;
    }
    
    // Check if set is admissible
    bool is_admissible(const std::unordered_set<std::string>& set) {
        if (!is_conflict_free(set)) {
            return false;
        }
        
        for (const auto& arg : set) {
            if (!is_acceptable(arg, set)) {
                return false;
            }
        }
        
        return true;
    }
    
    // Argument strength considering attacks
    float compute_strength(const std::string& arg_id) {
        if (!arguments_.count(arg_id)) {
            return 0.0f;
        }
        
        float base_strength = arguments_[arg_id].strength;
        float total_attack = 0.0f;
        
        for (const auto& attack : attacks_) {
            if (attack.target_id == arg_id) {
                float attacker_strength = compute_strength_recursive(attack.attacker_id, 3);
                total_attack += attacker_strength * attack.strength;
            }
        }
        
        return base_strength * (1.0f / (1.0f + total_attack));
    }
    
private:
    std::unordered_map<std::string, Argument> arguments_;
    std::vector<Attack> attacks_;
    
    std::unordered_set<std::string> compute_grounded_extension() {
        std::unordered_set<std::string> extension;
        
        bool changed = true;
        int max_iterations = 100;
        int iteration = 0;
        
        while (changed && iteration < max_iterations) {
            changed = false;
            iteration++;
            
            for (const auto& [id, arg] : arguments_) {
                if (extension.count(id)) {
                    continue;
                }
                
                if (is_acceptable(id, extension)) {
                    extension.insert(id);
                    changed = true;
                }
            }
        }
        
        return extension;
    }
    
    std::vector<std::unordered_set<std::string>> compute_preferred_extensions() {
        std::vector<std::unordered_set<std::string>> preferred;
        
        // Start with grounded
        auto grounded = compute_grounded_extension();
        
        // Try to extend
        auto extensions = extend_set(grounded);
        
        // Keep maximal admissible sets
        for (const auto& ext : extensions) {
            if (is_admissible(ext)) {
                bool is_maximal = true;
                
                for (const auto& other : extensions) {
                    if (other.size() > ext.size() &&
                        includes_set(other, ext) &&
                        is_admissible(other)) {
                        is_maximal = false;
                        break;
                    }
                }
                
                if (is_maximal) {
                    preferred.push_back(ext);
                }
            }
        }
        
        if (preferred.empty()) {
            preferred.push_back(grounded);
        }
        
        return preferred;
    }
    
    std::vector<std::unordered_set<std::string>> compute_stable_extensions() {
        std::vector<std::unordered_set<std::string>> stable;
        
        // A stable extension attacks all arguments outside it
        auto all_sets = generate_all_subsets();
        
        for (const auto& set : all_sets) {
            if (!is_conflict_free(set)) {
                continue;
            }
            
            // Check if it attacks all outside arguments
            bool is_stable = true;
            
            for (const auto& [id, arg] : arguments_) {
                if (!set.count(id)) {
                    // Must be attacked by set
                    if (!is_attacked_by(id, set)) {
                        is_stable = false;
                        break;
                    }
                }
            }
            
            if (is_stable) {
                stable.push_back(set);
            }
        }
        
        return stable;
    }
    
    bool attacks_relation(const std::string& from, const std::string& to) {
        for (const auto& attack : attacks_) {
            if (attack.attacker_id == from && attack.target_id == to) {
                return true;
            }
        }
        return false;
    }
    
    bool is_attacked_by(const std::string& arg_id,
                       const std::unordered_set<std::string>& set) {
        for (const auto& attacker : set) {
            if (attacks_relation(attacker, arg_id)) {
                return true;
            }
        }
        return false;
    }
    
    float compute_strength_recursive(const std::string& arg_id, int depth) {
        if (depth <= 0 || !arguments_.count(arg_id)) {
            return 0.0f;
        }
        
        float base = arguments_[arg_id].strength;
        float attack = 0.0f;
        
        for (const auto& att : attacks_) {
            if (att.target_id == arg_id) {
                attack += compute_strength_recursive(att.attacker_id, depth - 1) * att.strength;
            }
        }
        
        return base * (1.0f / (1.0f + attack * 0.5f));
    }
    
    std::vector<std::unordered_set<std::string>> generate_all_subsets() {
        std::vector<std::unordered_set<std::string>> subsets;
        std::vector<std::string> arg_ids;
        
        for (const auto& [id, _] : arguments_) {
            arg_ids.push_back(id);
        }
        
        // Generate up to 2^n subsets (limited for performance)
        int max_subsets = std::min(100, 1 << arg_ids.size());
        
        for (int mask = 0; mask < max_subsets; mask++) {
            std::unordered_set<std::string> subset;
            
            for (size_t i = 0; i < arg_ids.size(); i++) {
                if (mask & (1 << i)) {
                    subset.insert(arg_ids[i]);
                }
            }
            
            subsets.push_back(subset);
        }
        
        return subsets;
    }
    
    std::vector<std::unordered_set<std::string>> 
    extend_set(const std::unordered_set<std::string>& base) {
        std::vector<std::unordered_set<std::string>> extensions;
        extensions.push_back(base);
        
        for (const auto& [id, arg] : arguments_) {
            if (!base.count(id)) {
                auto extended = base;
                extended.insert(id);
                extensions.push_back(extended);
            }
        }
        
        return extensions;
    }
    
    bool includes_set(const std::unordered_set<std::string>& larger,
                     const std::unordered_set<std::string>& smaller) {
        for (const auto& elem : smaller) {
            if (!larger.count(elem)) {
                return false;
            }
        }
        return true;
    }
};

} // namespace dnn::reasoning
