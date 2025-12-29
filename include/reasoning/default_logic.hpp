#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace dnn::reasoning {

// Default Logic - reasoning with defaults and exceptions
class DefaultLogic {
public:
    struct Default {
        std::vector<std::string> prerequisites;
        std::vector<std::string> justifications;
        std::string conclusion;
        
        Default() {}
        Default(const std::vector<std::string>& pre, 
               const std::vector<std::string>& just,
               const std::string& concl)
            : prerequisites(pre), justifications(just), conclusion(concl) {}
    };
    
    struct Extension {
        std::unordered_set<std::string> beliefs;
        
        bool contains(const std::string& fact) const {
            return beliefs.count(fact) > 0;
        }
    };
    
    DefaultLogic() {}
    
    // Add default rule: pre : just / concl
    void add_default(const Default& def) {
        defaults_.push_back(def);
    }
    
    // Add hard fact
    void add_fact(const std::string& fact) {
        facts_.insert(fact);
    }
    
    // Compute preferred extension
    Extension compute_extension() {
        Extension ext;
        ext.beliefs = facts_;
        
        bool changed = true;
        int max_iterations = 100;
        int iteration = 0;
        
        while (changed && iteration < max_iterations) {
            changed = false;
            iteration++;
            
            for (const auto& def : defaults_) {
                // Check if default is applicable
                if (is_applicable(def, ext)) {
                    // Apply default
                    if (ext.beliefs.insert(def.conclusion).second) {
                        changed = true;
                    }
                }
            }
        }
        
        return ext;
    }
    
    // Check if conclusion follows from facts + defaults
    bool entails(const std::string& conclusion) {
        auto ext = compute_extension();
        return ext.contains(conclusion);
    }
    
    // Find all extensions (multiple possible)
    std::vector<Extension> find_all_extensions() {
        std::vector<Extension> extensions;
        
        // Start with facts
        Extension base_ext;
        base_ext.beliefs = facts_;
        
        // Try different orders of default application
        std::vector<int> default_order;
        for (size_t i = 0; i < defaults_.size(); i++) {
            default_order.push_back(i);
        }
        
        // Generate different extensions
        extensions.push_back(compute_extension_with_order(default_order));
        
        // Try reverse order
        std::reverse(default_order.begin(), default_order.end());
        auto ext2 = compute_extension_with_order(default_order);
        
        // Add if different
        if (!same_extension(extensions[0], ext2)) {
            extensions.push_back(ext2);
        }
        
        return extensions;
    }
    
    // Credulous reasoning: true in at least one extension
    bool credulous_entails(const std::string& conclusion) {
        auto extensions = find_all_extensions();
        
        for (const auto& ext : extensions) {
            if (ext.contains(conclusion)) {
                return true;
            }
        }
        
        return false;
    }
    
    // Skeptical reasoning: true in all extensions
    bool skeptical_entails(const std::string& conclusion) {
        auto extensions = find_all_extensions();
        
        if (extensions.empty()) {
            return false;
        }
        
        for (const auto& ext : extensions) {
            if (!ext.contains(conclusion)) {
                return false;
            }
        }
        
        return true;
    }
    
private:
    std::vector<Default> defaults_;
    std::unordered_set<std::string> facts_;
    
    bool is_applicable(const Default& def, const Extension& ext) {
        // Prerequisites must be satisfied
        for (const auto& pre : def.prerequisites) {
            if (!ext.contains(pre)) {
                return false;
            }
        }
        
        // Justifications must be consistent (not contradicted)
        for (const auto& just : def.justifications) {
            std::string negated = "NOT_" + just;
            if (ext.contains(negated)) {
                return false;  // Justification contradicted
            }
        }
        
        return true;
    }
    
    Extension compute_extension_with_order(const std::vector<int>& order) {
        Extension ext;
        ext.beliefs = facts_;
        
        for (int idx : order) {
            if (idx >= 0 && idx < static_cast<int>(defaults_.size())) {
                const auto& def = defaults_[idx];
                if (is_applicable(def, ext)) {
                    ext.beliefs.insert(def.conclusion);
                }
            }
        }
        
        return ext;
    }
    
    bool same_extension(const Extension& e1, const Extension& e2) const {
        return e1.beliefs == e2.beliefs;
    }
};

} // namespace dnn::reasoning
