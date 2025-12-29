#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <limits>

namespace dnn::reasoning {

// Monte Carlo Tree Search for game playing and planning
class MonteCarloTreeSearch {
public:
    struct State {
        std::vector<float> features;
        bool terminal;
        float reward;
        
        State() : terminal(false), reward(0.0f) {}
    };
    
    struct Node {
        State state;
        int visits;
        float total_reward;
        int parent_action;
        std::vector<int> children;  // Indices to child nodes
        std::vector<int> untried_actions;
        
        Node() : visits(0), total_reward(0.0f), parent_action(-1) {}
        
        float uct_value(int parent_visits, float exploration = 1.414f) const {
            if (visits == 0) return std::numeric_limits<float>::infinity();
            return (total_reward / visits) + 
                   exploration * std::sqrt(std::log(parent_visits) / visits);
        }
    };
    
    using ActionFunction = std::function<std::vector<int>(const State&)>;
    using TransitionFunction = std::function<State(const State&, int)>;
    using RewardFunction = std::function<float(const State&)>;
    
    MonteCarloTreeSearch(int num_simulations = 1000)
        : num_simulations_(num_simulations), gen_(std::random_device{}()) {}
    
    // Run MCTS and return best action
    int search(const State& root_state, 
              ActionFunction get_actions,
              TransitionFunction apply_action,
              RewardFunction get_reward) {
        
        nodes_.clear();
        nodes_.push_back(Node());
        nodes_[0].state = root_state;
        nodes_[0].untried_actions = get_actions(root_state);
        
        for (int sim = 0; sim < num_simulations_; sim++) {
            // Selection
            int node_idx = 0;
            while (!nodes_[node_idx].state.terminal && 
                   nodes_[node_idx].untried_actions.empty() &&
                   !nodes_[node_idx].children.empty()) {
                node_idx = select_child(node_idx);
            }
            
            // Expansion
            if (!nodes_[node_idx].untried_actions.empty()) {
                node_idx = expand(node_idx, get_actions, apply_action);
            }
            
            // Simulation (rollout)
            float reward = simulate(nodes_[node_idx].state, get_actions, 
                                   apply_action, get_reward);
            
            // Backpropagation
            backpropagate(node_idx, reward);
        }
        
        // Return action with most visits
        return best_action(0);
    }
    
private:
    int num_simulations_;
    std::vector<Node> nodes_;
    std::mt19937 gen_;
    
    int select_child(int node_idx) {
        const Node& node = nodes_[node_idx];
        int best_child = node.children[0];
        float best_uct = nodes_[best_child].uct_value(node.visits);
        
        for (int child_idx : node.children) {
            float uct = nodes_[child_idx].uct_value(node.visits);
            if (uct > best_uct) {
                best_uct = uct;
                best_child = child_idx;
            }
        }
        
        return best_child;
    }
    
    int expand(int node_idx, ActionFunction get_actions, TransitionFunction apply_action) {
        Node& node = nodes_[node_idx];
        
        // Pick random untried action
        std::uniform_int_distribution<> dist(0, node.untried_actions.size() - 1);
        int action_idx = dist(gen_);
        int action = node.untried_actions[action_idx];
        
        // Remove from untried
        node.untried_actions.erase(node.untried_actions.begin() + action_idx);
        
        // Create child node
        Node child;
        child.state = apply_action(node.state, action);
        child.parent_action = action;
        child.untried_actions = get_actions(child.state);
        
        int child_idx = nodes_.size();
        nodes_.push_back(child);
        nodes_[node_idx].children.push_back(child_idx);
        
        return child_idx;
    }
    
    float simulate(State state, ActionFunction get_actions, 
                  TransitionFunction apply_action, RewardFunction get_reward) {
        int depth = 0;
        const int max_depth = 50;
        
        while (!state.terminal && depth < max_depth) {
            auto actions = get_actions(state);
            if (actions.empty()) break;
            
            // Random rollout
            int action = actions[gen_() % actions.size()];
            state = apply_action(state, action);
            depth++;
        }
        
        return get_reward(state);
    }
    
    void backpropagate(int node_idx, float reward) {
        while (node_idx >= 0) {
            nodes_[node_idx].visits++;
            nodes_[node_idx].total_reward += reward;
            
            // Find parent
            int parent_idx = -1;
            for (int i = 0; i < static_cast<int>(nodes_.size()); i++) {
                for (int child : nodes_[i].children) {
                    if (child == node_idx) {
                        parent_idx = i;
                        break;
                    }
                }
                if (parent_idx >= 0) break;
            }
            
            node_idx = parent_idx;
        }
    }
    
    int best_action(int node_idx) const {
        const Node& node = nodes_[node_idx];
        if (node.children.empty()) return -1;
        
        int best_child = node.children[0];
        int max_visits = nodes_[best_child].visits;
        
        for (int child_idx : node.children) {
            if (nodes_[child_idx].visits > max_visits) {
                max_visits = nodes_[child_idx].visits;
                best_child = child_idx;
            }
        }
        
        return nodes_[best_child].parent_action;
    }
};

} // namespace dnn::reasoning
