#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <iostream>

struct PlanNode {
    std::string action;
    double probability;
    int visits = 0;
    double value = 0.0;
    std::vector<std::unique_ptr<PlanNode>> children;
    PlanNode* parent = nullptr;
    
    PlanNode(const std::string& a, double p, PlanNode* pr = nullptr) 
        : action(a), probability(p), parent(pr) {}
    
    double get_ucb(double total_visits) const {
        if (visits == 0) return 1e9; // Infinity for unvisited nodes
        return (value / visits) + 1.41 * std::sqrt(std::log(total_visits) / visits);
    }
};

class PlanningUnit {
public:
    PlanningUnit();
    std::string decide_best_action(const std::string& context, double energy, double boredom);
    
private:
    std::unique_ptr<PlanNode> root;
    void build_tree();
    
    // MCTS Methods
    PlanNode* select(PlanNode* node);
    void expand(PlanNode* node);
    double simulate(PlanNode* node, double energy, double boredom);
    void backpropagate(PlanNode* node, double reward);
};
