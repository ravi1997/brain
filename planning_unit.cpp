#include "planning_unit.hpp"
#include <algorithm>
#include <random>
#include <iostream>

PlanningUnit::PlanningUnit() {
    build_tree();
}

void PlanningUnit::build_tree() {
    root = std::make_unique<PlanNode>("ROOT", 1.0);
    // Tree is now built dynamically during MCTS expansion
}

std::vector<Goal> PlanningUnit::generate_goals(double boredom, double curiosity, const std::string& recent_topic) {
    std::vector<Goal> new_goals;
    static int goal_counter = 1000;

    // High Boredom + High Curiosity -> Self-directed Research
    if (boredom > 0.6 && curiosity > 0.5) {
        Goal g;
        g.id = goal_counter++;
        g.description = "Research " + (recent_topic.empty() ? "random_physics" : recent_topic);
        g.priority = static_cast<int>(boredom * 100);
        g.status = "PENDING";
        g.type = "RESEARCH";
        new_goals.push_back(g);
        active_goals.push_back(g);
    }

    // High Boredom + Low Curiosity -> Maintenance / Cleanup
    if (boredom > 0.8 && curiosity < 0.3) {
        Goal g;
        g.id = goal_counter++;
        g.description = "Organize Memory Graph";
        g.priority = 60;
        g.status = "PENDING";
        g.type = "MAINTENANCE";
        new_goals.push_back(g);
        active_goals.push_back(g);
    }

    return new_goals;
}

std::string PlanningUnit::decide_best_action(const std::string& context, double energy, double boredom) {
    // Run MCTS for 200 iterations (increased for detailed search)
    for (int i = 0; i < 200; ++i) {
        PlanNode* selected = select(root.get());
        
        // Dynamic Expansion
        if (selected->visits > 3 && selected->children.empty()) { // Expand if visited enough
            expand(selected);
            if (!selected->children.empty()) {
                selected = selected->children[0].get(); // Move to new child
            }
        }
        
        double reward = simulate(selected, energy, boredom);
        backpropagate(selected, reward);
    }
    
    // Pruning: Remove nodes with very low visits to save memory (long-running optim)
    // For now, just picking best action
    
    if (root->children.empty()) {
        // Fallback if no expansion happened (shouldn't happen with 200 iters)
        expand(root.get()); 
    }

    if (root->children.empty()) return "IDLE";
    
    PlanNode* best = root->children[0].get();
    for (const auto& child : root->children) {
        if (child->visits > best->visits) {
            best = child.get();
        }
    }

    // Drill down
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

void PlanningUnit::expand(PlanNode* node) {
    // Logic to generate children based on node type
    if (node->action == "ROOT") {
        node->children.push_back(std::make_unique<PlanNode>("RESEARCH", 0.3, node));
        node->children.push_back(std::make_unique<PlanNode>("INTERACT", 0.3, node));
        node->children.push_back(std::make_unique<PlanNode>("SLEEP", 0.2, node));
        node->children.push_back(std::make_unique<PlanNode>("IDLE", 0.2, node));
    } else if (node->action == "RESEARCH") {
        node->children.push_back(std::make_unique<PlanNode>("DEEP_SCAN", 0.6, node));
        node->children.push_back(std::make_unique<PlanNode>("BROWSING", 0.4, node));
    } else if (node->action == "INTERACT") {
        node->children.push_back(std::make_unique<PlanNode>("ASK_QUESTION", 0.5, node));
        node->children.push_back(std::make_unique<PlanNode>("PROVIDE_INFO", 0.5, node));
    }
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
