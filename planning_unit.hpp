#pragma once
#include <string>
#include <vector>
#include <memory>

struct PlanNode {
    std::string action;
    double probability;
    std::vector<std::unique_ptr<PlanNode>> children;
    
    PlanNode(const std::string& a, double p) : action(a), probability(p) {}
};

class PlanningUnit {
public:
    PlanningUnit();
    std::string decide_best_action(const std::string& context);
    
private:
    std::unique_ptr<PlanNode> root;
    void build_tree();
};
