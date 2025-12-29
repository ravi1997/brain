#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace dnn::reasoning {

// Automated Theorem Proving using resolution
class AutomatedTheoremProving {
public:
    struct Literal {
        std::string predicate;
        std::vector<std::string> terms;
        bool negated;
        
        Literal() : negated(false) {}
        Literal(const std::string& p, const std::vector<std::string>& t, bool neg = false)
            : predicate(p), terms(t), negated(neg) {}
        
        std::string to_string() const {
            std::string s = negated ? "¬" : "";
            s += predicate + "(";
            for (size_t i = 0; i < terms.size(); i++) {
                s += terms[i];
                if (i < terms.size() - 1) s += ",";
            }
            s += ")";
            return s;
        }
    };
    
    struct Clause {
        std::vector<Literal> literals;
        
        Clause() {}
        Clause(const std::vector<Literal>& lits) : literals(lits) {}
        
        bool is_empty() const { return literals.empty(); }
        
        std::string to_string() const {
            if (literals.empty()) return "□";  // Empty clause (contradiction)
            
            std::string s;
            for (size_t i = 0; i < literals.size(); i++) {
                s += literals[i].to_string();
                if (i < literals.size() - 1) s += " ∨ ";
            }
            return s;
        }
    };
    
    AutomatedTheoremProving() {}
    
    // Add axiom (clause to knowledge base)
    void add_axiom(const Clause& clause) {
        knowledge_base_.push_back(clause);
    }
    
    // Prove theorem using resolution
    bool prove(const Clause& goal) {
        // Convert goal to negated form
        std::vector<Clause> working_set = knowledge_base_;
        
        // Add negation of goal
        Clause negated_goal;
        for (const auto& lit : goal.literals) {
            Literal neg_lit = lit;
            neg_lit.negated = !neg_lit.negated;
            negated_goal.literals.push_back(neg_lit);
        }
        working_set.push_back(negated_goal);
        
        // Resolution proof search
        int max_iterations = 1000;
        for (int iter = 0; iter < max_iterations; iter++) {
            std::vector<Clause> new_clauses;
            
            // Try to resolve pairs of clauses
            for (size_t i = 0; i < working_set.size(); i++) {
                for (size_t j = i + 1; j < working_set.size(); j++) {
                    auto resolvents = resolve(working_set[i], working_set[j]);
                    
                    for (const auto& resolvent : resolvents) {
                        // Found empty clause - proof by contradiction
                        if (resolvent.is_empty()) {
                            return true;
                        }
                        
                        // Add new clause if not already present
                        if (!contains_clause(working_set, resolvent) &&
                            !contains_clause(new_clauses, resolvent)) {
                            new_clauses.push_back(resolvent);
                        }
                    }
                }
            }
            
            if (new_clauses.empty()) {
                break;  // No new clauses, proof failed
            }
            
            working_set.insert(working_set.end(), new_clauses.begin(), new_clauses.end());
        }
        
        return false;
    }
    
    // Check if formula is satisfiable
    bool is_satisfiable(const std::vector<Clause>& formula) {
        // Try to derive empty clause
        std::vector<Clause> working_set = formula;
        
        for (int iter = 0; iter < 500; iter++) {
            std::vector<Clause> new_clauses;
            
            for (size_t i = 0; i < working_set.size(); i++) {
                for (size_t j = i + 1; j < working_set.size(); j++) {
                    auto resolvents = resolve(working_set[i], working_set[j]);
                    
                    for (const auto& res : resolvents) {
                        if (res.is_empty()) {
                            return false;  // Unsatisfiable
                        }
                        if (!contains_clause(working_set, res)) {
                            new_clauses.push_back(res);
                        }
                    }
                }
            }
            
            if (new_clauses.empty()) {
                return true;  // Satisfiable
            }
            
            working_set.insert(working_set.end(), new_clauses.begin(), new_clauses.end());
        }
        
        return true;  // Assume satisfiable if no contradiction found
    }
    
private:
    std::vector<Clause> knowledge_base_;
    
    // Resolution rule: resolve two clauses
    std::vector<Clause> resolve(const Clause& c1, const Clause& c2) {
        std::vector<Clause> resolvents;
        
        // Try to find complementary literals
        for (size_t i = 0; i < c1.literals.size(); i++) {
            for (size_t j = 0; j < c2.literals.size(); j++) {
                if (can_resolve(c1.literals[i], c2.literals[j])) {
                    // Create resolvent
                    Clause resolvent;
                    
                    // Add all literals from c1 except i
                    for (size_t k = 0; k < c1.literals.size(); k++) {
                        if (k != i) {
                            resolvent.literals.push_back(c1.literals[k]);
                        }
                    }
                    
                    // Add all literals from c2 except j
                    for (size_t k = 0; k < c2.literals.size(); k++) {
                        if (k != j) {
                            resolvent.literals.push_back(c2.literals[k]);
                        }
                    }
                    
                    resolvents.push_back(resolvent);
                }
            }
        }
        
        return resolvents;
    }
    
    bool can_resolve(const Literal& l1, const Literal& l2) {
        // Literals must be complementary (one negated, one not)
        if (l1.negated == l2.negated) {
            return false;
        }
        
        // Predicates and terms must match
        if (l1.predicate != l2.predicate) {
            return false;
        }
        
        if (l1.terms.size() != l2.terms.size()) {
            return false;
        }
        
        for (size_t i = 0; i < l1.terms.size(); i++) {
            // Simplified: exact match or one is variable
            if (l1.terms[i] != l2.terms[i] &&
                !is_variable(l1.terms[i]) &&
                !is_variable(l2.terms[i])) {
                return false;
            }
        }
        
        return true;
    }
    
    bool is_variable(const std::string& term) const {
        return !term.empty() && std::isupper(term[0]);
    }
    
    bool contains_clause(const std::vector<Clause>& clauses, const Clause& clause) const {
        for (const auto& c : clauses) {
            if (c.literals.size() == clause.literals.size()) {
                bool match = true;
                for (size_t i = 0; i < c.literals.size(); i++) {
                    if (c.literals[i].predicate != clause.literals[i].predicate ||
                        c.literals[i].negated != clause.literals[i].negated) {
                        match = false;
                        break;
                    }
                }
                if (match) return true;
            }
        }
        return false;
    }
};

} // namespace dnn::reasoning
