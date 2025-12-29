#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>

namespace dnn::reasoning {

// Constraint Satisfaction Problem solver
class CSPSolver {
public:
    using Domain = std::vector<int>;
    using Variable = std::string;
    using ConstraintFunction = std::function<bool(int, int)>;
    
    struct Constraint {
        Variable var1;
        Variable var2;
        ConstraintFunction check;
    };
    
    CSPSolver() {}
    
    // Add variable with domain
    void add_variable(const Variable& var, const Domain& domain) {
        domains_[var] = domain;
        assignment_.erase(var);
    }
    
    // Add binary constraint between two variables
    void add_constraint(const Variable& var1, const Variable& var2, 
                       ConstraintFunction constraint) {
        constraints_.push_back({var1, var2, constraint});
    }
    
    // Solve CSP using backtracking
    bool solve() {
        assignment_.clear();
        return backtrack();
    }
    
    // Get solution
    std::unordered_map<Variable, int> get_solution() const {
        return assignment_;
    }
    
private:
    std::unordered_map<Variable, Domain> domains_;
    std::vector<Constraint> constraints_;
    std::unordered_map<Variable, int> assignment_;
    
    bool backtrack() {
        // Check if assignment is complete
        if (assignment_.size() == domains_.size()) {
            return true;
        }
        
        // Select unassigned variable
        Variable var = select_unassigned_variable();
        
        // Try each value in domain
        for (int value : domains_[var]) {
            if (is_consistent(var, value)) {
                assignment_[var] = value;
                
                if (backtrack()) {
                    return true;
                }
                
                assignment_.erase(var);
            }
        }
        
        return false;
    }
    
    Variable select_unassigned_variable() const {
        // MRV (Minimum Remaining Values) heuristic
        Variable best_var;
        int min_domain_size = INT_MAX;
        
        for (const auto& [var, domain] : domains_) {
            if (assignment_.find(var) == assignment_.end()) {
                int valid_count = count_valid_values(var);
                if (valid_count < min_domain_size) {
                    min_domain_size = valid_count;
                    best_var = var;
                }
            }
        }
        
        return best_var;
    }
    
    int count_valid_values(const Variable& var) const {
        int count = 0;
        for (int value : domains_.at(var)) {
            if (is_consistent(var, value)) {
                count++;
            }
        }
        return count;
    }
    
    bool is_consistent(const Variable& var, int value) const {
        // Check all constraints
        for (const auto& constraint : constraints_) {
            bool var1_match = (constraint.var1 == var);
            bool var2_match = (constraint.var2 == var);
            
            if (var1_match) {
                auto it = assignment_.find(constraint.var2);
                if (it != assignment_.end()) {
                    if (!constraint.check(value, it->second)) {
                        return false;
                    }
                }
            } else if (var2_match) {
                auto it = assignment_.find(constraint.var1);
                if (it != assignment_.end()) {
                    if (!constraint.check(it->second, value)) {
                        return false;
                    }
                }
            }
        }
        
        return true;
    }
};

} // namespace dnn::reasoning
