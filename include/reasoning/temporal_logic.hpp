#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <algorithm>

namespace dnn::reasoning {

// Temporal Logic Reasoning with Linear Temporal Logic (LTL)
class TemporalLogicReasoning {
public:
    enum class TemporalOperator {
        NEXT,        // X p: p holds in the next state
        EVENTUALLY,  // F p: p holds at some future state
        ALWAYS,      // G p: p holds in all future states
        UNTIL        // p U q: p holds until q becomes true
    };
    
    struct TemporalFormula {
        TemporalOperator op;
        std::string proposition;
        std::shared_ptr<TemporalFormula> left;
        std::shared_ptr<TemporalFormula> right;
        bool is_atomic;
        
        TemporalFormula() : is_atomic(true) {}
        
        // Atomic proposition
        static std::shared_ptr<TemporalFormula> atom(const std::string& prop) {
            auto f = std::make_shared<TemporalFormula>();
            f->proposition = prop;
            f->is_atomic = true;
            return f;
        }
        
        // Temporal operators
        static std::shared_ptr<TemporalFormula> next(std::shared_ptr<TemporalFormula> p) {
            auto f = std::make_shared<TemporalFormula>();
            f->op = TemporalOperator::NEXT;
            f->left = p;
            f->is_atomic = false;
            return f;
        }
        
        static std::shared_ptr<TemporalFormula> eventually(std::shared_ptr<TemporalFormula> p) {
            auto f = std::make_shared<TemporalFormula>();
            f->op = TemporalOperator::EVENTUALLY;
            f->left = p;
            f->is_atomic = false;
            return f;
        }
        
        static std::shared_ptr<TemporalFormula> always(std::shared_ptr<TemporalFormula> p) {
            auto f = std::make_shared<TemporalFormula>();
            f->op = TemporalOperator::ALWAYS;
            f->left = p;
            f->is_atomic = false;
            return f;
        }
        
        static std::shared_ptr<TemporalFormula> until(std::shared_ptr<TemporalFormula> p,
                                                      std::shared_ptr<TemporalFormula> q) {
            auto f = std::make_shared<TemporalFormula>();
            f->op = TemporalOperator::UNTIL;
            f->left = p;
            f->right = q;
            f->is_atomic = false;
            return f;
        }
    };
    
    struct State {
        std::unordered_set<std::string> true_propositions;
        int timestamp;
        
        State() : timestamp(0) {}
        
        bool holds(const std::string& prop) const {
            return true_propositions.count(prop) > 0;
        }
    };
    
    using Trace = std::vector<State>;
    
    TemporalLogicReasoning() {}
    
    // Check if formula holds on a trace starting at position
    bool check_formula(const std::shared_ptr<TemporalFormula>& formula,
                      const Trace& trace, int position = 0) {
        if (!formula) return false;
        
        if (position >= static_cast<int>(trace.size())) {
            return false;
        }
        
        if (formula->is_atomic) {
            return trace[position].holds(formula->proposition);
        }
        
        switch (formula->op) {
            case TemporalOperator::NEXT:
                // X p: p holds in next state
                if (position + 1 < static_cast<int>(trace.size())) {
                    return check_formula(formula->left, trace, position + 1);
                }
                return false;
                
            case TemporalOperator::EVENTUALLY:
                // F p: p holds at some future state
                for (int i = position; i < static_cast<int>(trace.size()); i++) {
                    if (check_formula(formula->left, trace, i)) {
                        return true;
                    }
                }
                return false;
                
            case TemporalOperator::ALWAYS:
                // G p: p holds in all future states
                for (int i = position; i < static_cast<int>(trace.size()); i++) {
                    if (!check_formula(formula->left, trace, i)) {
                        return false;
                    }
                }
                return true;
                
            case TemporalOperator::UNTIL:
                // p U q: p holds until q becomes true
                for (int i = position; i < static_cast<int>(trace.size()); i++) {
                    if (check_formula(formula->right, trace, i)) {
                        return true;  // q holds
                    }
                    if (!check_formula(formula->left, trace, i)) {
                        return false;  // p doesn't hold before q
                    }
                }
                return false;
                
            default:
                return false;
        }
    }
    
    // Model checking: verify property on all traces
    bool model_check(const std::shared_ptr<TemporalFormula>& formula,
                    const std::vector<Trace>& traces) {
        for (const auto& trace : traces) {
            if (!check_formula(formula, trace, 0)) {
                return false;
            }
        }
        return true;
    }
    
    // Generate trace from state sequence
    Trace create_trace(const std::vector<std::unordered_set<std::string>>& state_sequence) {
        Trace trace;
        
        for (size_t i = 0; i < state_sequence.size(); i++) {
            State state;
            state.true_propositions = state_sequence[i];
            state.timestamp = i;
            trace.push_back(state);
        }
        
        return trace;
    }
    
    // Find violations of formula in trace
    std::vector<int> find_violations(const std::shared_ptr<TemporalFormula>& formula,
                                    const Trace& trace) {
        std::vector<int> violations;
        
        for (int i = 0; i < static_cast<int>(trace.size()); i++) {
            if (!check_formula(formula, trace, i)) {
                violations.push_back(i);
            }
        }
        
        return violations;
    }
    
    // Safety property: something bad never happens
    bool check_safety(const std::string& bad_proposition, const Trace& trace) {
        auto bad = TemporalFormula::atom(bad_proposition);
        auto never_bad = TemporalFormula::always(bad);  // Negation implicit
        
        // Check that bad never holds
        for (const auto& state : trace) {
            if (state.holds(bad_proposition)) {
                return false;
            }
        }
        return true;
    }
    
    // Liveness property: something good eventually happens
    bool check_liveness(const std::string& good_proposition, const Trace& trace) {
        auto good = TemporalFormula::atom(good_proposition);
        auto eventually_good = TemporalFormula::eventually(good);
        
        return check_formula(eventually_good, trace, 0);
    }
};

} // namespace dnn::reasoning
