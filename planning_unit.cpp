#include "planning_unit.hpp"
#include <algorithm>
#include <random>

PlanningUnit::PlanningUnit() {
    build_tree();
}

void PlanningUnit::build_tree() {
    root = std::make_unique<PlanNode>("ROOT", 1.0);
    
    auto research = std::make_unique<PlanNode>("RESEARCH", 0.4);
    research->children.push_back(std::make_unique<PlanNode>("DEEP_SCAN", 0.6));
    research->children.push_back(std::make_unique<PlanNode>("BROWSING", 0.4));
    
    auto interact = std::make_unique<PlanNode>("INTERACT", 0.4);
    interact->children.push_back(std::make_unique<PlanNode>("ASK_QUESTION", 0.5));
    interact->children.push_back(std::make_unique<PlanNode>("PROVIDE_INFO", 0.5));
    
    auto idle = std::make_unique<PlanNode>("IDLE", 0.2);
    
    root->children.push_back(std::move(research));
    root->children.push_back(std::move(interact));
    root->children.push_back(std::move(idle));
}

std::string PlanningUnit::decide_best_action(const std::string& context) {
    // Simple traversal for Phase 1
    if (!root || root->children.empty()) return "IDLE";
    
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dis(0, 1);
    double r = dis(gen);
    
    double cumulative = 0.0;
    for (const auto& node : root->children) {
        cumulative += node->probability;
        if (r <= cumulative) {
            if (node->children.empty()) return node->action;
            // Go one level deeper
            double r2 = dis(gen);
            double c2 = 0.0;
            for (const auto& child : node->children) {
                c2 += child->probability;
                if (r2 <= c2) return child->action;
            }
            return node->action;
        }
    }
    
    return "IDLE";
}
