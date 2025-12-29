#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <queue>
#include <algorithm>

namespace dnn::reasoning {

// Hierarchical Task Network (HTN) Planning
class HTNPlanner {
public:
    struct Task {
        std::string name;
        bool is_primitive;  // Primitive (directly executable) or compound
        std::unordered_map<std::string, std::string> parameters;
        
        Task() : is_primitive(true) {}
        Task(const std::string& n, bool prim = true) 
            : name(n), is_primitive(prim) {}
    };
    
    struct Method {
        std::string name;
        std::string compound_task;  // Which compound task this decomposes
        std::vector<std::string> preconditions;
        std::vector<Task> subtasks;
        
        Method() {}
        Method(const std::string& n, const std::string& task)
            : name(n), compound_task(task) {}
    };
    
    struct Action {
        std::string name;
        std::vector<std::string> preconditions;
        std::vector<std::string> add_effects;
        std::vector<std::string> delete_effects;
        
        Action() {}
        Action(const std::string& n) : name(n) {}
    };
    
    struct State {
        std::unordered_set<std::string> facts;
        
        bool satisfies(const std::vector<std::string>& conditions) const {
            for (const auto& cond : conditions) {
                if (facts.find(cond) == facts.end()) {
                    return false;
                }
            }
            return true;
        }
        
        void apply_action(const Action& action) {
            // Add effects
            for (const auto& effect : action.add_effects) {
                facts.insert(effect);
            }
            // Delete effects
            for (const auto& effect : action.delete_effects) {
                facts.erase(effect);
            }
        }
    };
    
    HTNPlanner() {}
    
    // Add method for decomposing compound tasks
    void add_method(const Method& method) {
        methods_[method.compound_task].push_back(method);
    }
    
    // Add primitive action
    void add_action(const Action& action) {
        actions_[action.name] = action;
    }
    
    // Plan: decompose tasks and find action sequence
    std::vector<Action> plan(const std::vector<Task>& goals, State& initial_state) {
        std::vector<Action> plan;
        std::vector<Task> task_network = goals;
        State current_state = initial_state;
        
        return plan_recursive(task_network, current_state, plan);
    }
    
private:
    std::unordered_map<std::string, std::vector<Method>> methods_;
    std::unordered_map<std::string, Action> actions_;
    
    std::vector<Action> plan_recursive(std::vector<Task>& tasks, State& state, 
                                       std::vector<Action>& current_plan) {
        // Base case: no more tasks
        if (tasks.empty()) {
            return current_plan;
        }
        
        // Get first task
        Task task = tasks.front();
        tasks.erase(tasks.begin());
        
        if (task.is_primitive) {
            // Execute primitive task (action)
            if (actions_.count(task.name)) {
                const Action& action = actions_[task.name];
                
                // Check if preconditions are satisfied
                if (state.satisfies(action.preconditions)) {
                    // Apply action
                    state.apply_action(action);
                    current_plan.push_back(action);
                    
                    // Continue with remaining tasks
                    return plan_recursive(tasks, state, current_plan);
                } else {
                    // Preconditions not met - planning fails
                    return {};
                }
            } else {
                // Action not found
                return {};
            }
        } else {
            // Compound task: try decomposition methods
            if (methods_.count(task.name)) {
                for (const auto& method : methods_[task.name]) {
                    // Check method preconditions
                    if (state.satisfies(method.preconditions)) {
                        // Decompose: add subtasks to front of task list
                        std::vector<Task> new_tasks = method.subtasks;
                        new_tasks.insert(new_tasks.end(), tasks.begin(), tasks.end());
                        
                        // Try planning with decomposition
                        State state_copy = state;
                        std::vector<Action> plan_copy = current_plan;
                        
                        auto result = plan_recursive(new_tasks, state_copy, plan_copy);
                        
                        if (!result.empty() || new_tasks.empty()) {
                            // Success
                            state = state_copy;
                            return result;
                        }
                    }
                }
            }
            
            // No applicable method found
            return {};
        }
    }
    
public:
    // Verify plan correctness
    bool verify_plan(const std::vector<Action>& plan, const State& initial_state,
                    const std::vector<std::string>& goals) {
        State state = initial_state;
        
        // Execute plan
        for (const auto& action : plan) {
            if (!state.satisfies(action.preconditions)) {
                return false;  // Precondition violated
            }
            state.apply_action(action);
        }
        
        // Check if goals are achieved
        return state.satisfies(goals);
    }
    
    // Get plan length (number of actions)
    int get_plan_length(const std::vector<Action>& plan) const {
        return plan.size();
    }
    
    // Estimate plan cost (simplified: each action costs 1)
    float estimate_cost(const std::vector<Action>& plan) const {
        return static_cast<float>(plan.size());
    }
};

} // namespace dnn::reasoning
