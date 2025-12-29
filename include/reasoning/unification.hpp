#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace dnn::reasoning {

// Logical Unification for pattern matching in logic programming
class LogicalUnification {
public:
    struct Term {
        enum Type { CONSTANT, VARIABLE, FUNCTION };
        Type type;
        std::string value;
        std::vector<std::shared_ptr<Term>> arguments;
        
        Term() : type(CONSTANT) {}
        Term(Type t, const std::string& v) : type(t), value(v) {}
        
        bool is_variable() const { return type == VARIABLE; }
        bool is_constant() const { return type == CONSTANT; }
        bool is_function() const { return type == FUNCTION; }
        
        std::string to_string() const {
            if (is_constant() || is_variable()) {
                return value;
            }
            
            std::string s = value + "(";
            for (size_t i = 0; i < arguments.size(); i++) {
                s += arguments[i]->to_string();
                if (i < arguments.size() - 1) s += ",";
            }
            s += ")";
            return s;
        }
    };
    
    using Substitution = std::unordered_map<std::string, std::shared_ptr<Term>>;
    
    LogicalUnification() {}
    
    // Create variable term
    static std::shared_ptr<Term> var(const std::string& name) {
        return std::make_shared<Term>(Term::VARIABLE, name);
    }
    
    // Create constant term
    static std::shared_ptr<Term> constant(const std::string& name) {
        return std::make_shared<Term>(Term::CONSTANT, name);
    }
    
    // Create function term
    static std::shared_ptr<Term> function(const std::string& name, 
                                         const std::vector<std::shared_ptr<Term>>& args) {
        auto term = std::make_shared<Term>(Term::FUNCTION, name);
        term->arguments = args;
        return term;
    }
    
    // Unify two terms
    bool unify(const std::shared_ptr<Term>& t1, 
              const std::shared_ptr<Term>& t2,
              Substitution& subst) {
        // Apply current substitutions
        auto s1 = apply_substitution(t1, subst);
        auto s2 = apply_substitution(t2, subst);
        
        // Same term
        if (same_term(s1, s2)) {
            return true;
        }
        
        // Variable unification
        if (s1->is_variable()) {
            return unify_variable(s1, s2, subst);
        }
        
        if (s2->is_variable()) {
            return unify_variable(s2, s1, subst);
        }
        
        // Function unification
        if (s1->is_function() && s2->is_function()) {
            if (s1->value != s2->value || 
                s1->arguments.size() != s2->arguments.size()) {
                return false;
            }
            
            for (size_t i = 0; i < s1->arguments.size(); i++) {
                if (!unify(s1->arguments[i], s2->arguments[i], subst)) {
                    return false;
                }
            }
            
            return true;
        }
        
        return false;
    }
    
    // Pattern matching
    bool match(const std::shared_ptr<Term>& pattern,
              const std::shared_ptr<Term>& term,
              Substitution& bindings) {
        return unify(pattern, term, bindings);
    }
    
    // Apply substitution to term
    std::shared_ptr<Term> apply_substitution(const std::shared_ptr<Term>& term,
                                            const Substitution& subst) {
        if (!term) return nullptr;
        
        if (term->is_variable()) {
            if (subst.count(term->value)) {
                // Recursively apply (in case substitution contains variables)
                return apply_substitution(subst.at(term->value), subst);
            }
            return term;
        }
        
        if (term->is_constant()) {
            return term;
        }
        
        // Function: apply to all arguments
        auto result = std::make_shared<Term>(Term::FUNCTION, term->value);
        for (const auto& arg : term->arguments) {
            result->arguments.push_back(apply_substitution(arg, subst));
        }
        
        return result;
    }
    
    // Compose substitutions
    Substitution compose(const Substitution& s1, const Substitution& s2) {
        Substitution result = s2;
        
        for (const auto& [var, term] : s1) {
            result[var] = apply_substitution(term, s2);
        }
        
        return result;
    }
    
private:
    bool same_term(const std::shared_ptr<Term>& t1,
                  const std::shared_ptr<Term>& t2) {
        if (!t1 || !t2) return false;
        
        if (t1->type != t2->type) return false;
        if (t1->value != t2->value) return false;
        
        if (t1->is_function()) {
            if (t1->arguments.size() != t2->arguments.size()) return false;
            
            for (size_t i = 0; i < t1->arguments.size(); i++) {
                if (!same_term(t1->arguments[i], t2->arguments[i])) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    bool unify_variable(const std::shared_ptr<Term>& var,
                       const std::shared_ptr<Term>& term,
                       Substitution& subst) {
        if (!var || !term) return false;
        
        // Occurs check: prevent infinite structures
        if (occurs(var->value, term, subst)) {
            return false;
        }
        
        subst[var->value] = term;
        return true;
    }
    
    bool occurs(const std::string& var,
               const std::shared_ptr<Term>& term,
               const Substitution& subst) {
        if (!term) return false;
        
        auto t = apply_substitution(term, subst);
        
        if (t->is_variable()) {
            return t->value == var;
        }
        
        if (t->is_function()) {
            for (const auto& arg : t->arguments) {
                if (occurs(var, arg, subst)) {
                    return true;
                }
            }
        }
        
        return false;
    }
};

} // namespace dnn::reasoning
