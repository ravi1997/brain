#include "planning_unit.hpp"
#include <algorithm>
#include <random>
#include <iostream>

PlanningUnit::PlanningUnit() {
    build_tree();
}

void PlanningUnit::build_tree() {
    root = std::make_unique<PlanNode>("ROOT", 1.0);
    
    // Level 1
    auto research = std::make_unique<PlanNode>("RESEARCH", 0.3, root.get());
    auto interact = std::make_unique<PlanNode>("INTERACT", 0.3, root.get());
    auto sleep = std::make_unique<PlanNode>("SLEEP", 0.2, root.get());
    auto idle = std::make_unique<PlanNode>("IDLE", 0.2, root.get());
    
    root->children.push_back(std::move(research));
    root->children.push_back(std::move(interact));
    root->children.push_back(std::move(sleep));
    root->children.push_back(std::move(idle));

    // Level 2 for Research
    auto& r_node = root->children[0];
    r_node->children.push_back(std::make_unique<PlanNode>("DEEP_SCAN", 0.6, r_node.get()));
    r_node->children.push_back(std::make_unique<PlanNode>("BROWSING", 0.4, r_node.get()));

    // Level 2 for Interact
    auto& i_node = root->children[1];
    i_node->children.push_back(std::make_unique<PlanNode>("ASK_QUESTION", 0.5, i_node.get()));
    i_node->children.push_back(std::make_unique<PlanNode>("PROVIDE_INFO", 0.5, i_node.get()));
}

std::string PlanningUnit::decide_best_action(const std::string& context, double energy, double boredom) {
    // Run MCTS for 100 iterations
    for (int i = 0; i < 100; ++i) {
        PlanNode* selected = select(root.get());
        if (selected->visits > 0 && !selected->action.empty() && selected->children.empty()) {
            // Expansion might happen here if we had dynamic actions, 
            // but for now we follow the static tree structure.
        }
        double reward = simulate(selected, energy, boredom);
        backpropagate(selected, reward);
    }

    // Pick child with highest visits (robust child)
    if (root->children.empty()) return "IDLE";
    
    PlanNode* best = root->children[0].get();
    for (const auto& child : root->children) {
        if (child->visits > best->visits) {
            best = child.get();
        }
    }

    // If best has children, drill down
    while (!best->children.empty()) {
        PlanNode* next_best = best->children[0].get();
        for (const auto& child : best->children) {
            if (child->visits > next_best->visits) {
                next_best = child.get();
            }
        }
        best = next_best;
    }

    return best->action;
}

PlanNode* PlanningUnit::select(PlanNode* node) {
    while (!node->children.empty()) {
        bool all_visited = true;
        for (const auto& child : node->children) {
            if (child->visits == 0) {
                return child.get();
            }
        }
        
        // Use UCB
        PlanNode* best = node->children[0].get();
        double best_ucb = best->get_ucb(static_cast<double>(node->visits));
        
        for (const auto& child : node->children) {
            double ucb = child->get_ucb(static_cast<double>(node->visits));
            if (ucb > best_ucb) {
                best_ucb = ucb;
                best = child.get();
            }
        }
        node = best;
    }
    return node;
}

double PlanningUnit::simulate(PlanNode* node, double energy, double boredom) {
    // Heuristic rewards based on state (energy, boredom)
    if (node->action == "SLEEP") {
        return (1.0 - energy) * 2.0; // High reward if low energy
    }
    if (node->action == "RESEARCH" || node->action == "DEEP_SCAN" || node->action == "BROWSING") {
        return boredom * 1.5 + (energy > 0.5 ? 0.5 : 0.0); // Reward if bored and has energy
    }
    if (node->action == "INTERACT" || node->action == "ASK_QUESTION" || node->action == "PROVIDE_INFO") {
        return energy * 1.2; // Interaction needs energy
    }
    if (node->action == "IDLE") {
        return 0.2; // Small base reward
    }
    return 0.1;
}

void PlanningUnit::backpropagate(PlanNode* node, double reward) {
    while (node) {
        node->visits++;
        node->value += reward;
        node = node->parent;
    }
}
