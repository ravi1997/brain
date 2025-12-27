#include <gtest/gtest.h>
#include "../planning_unit.hpp"

class PlanningDynamicTest : public ::testing::Test {
protected:
    PlanningUnit planner;
};

TEST_F(PlanningDynamicTest, ExpandsDynamically) {
    // Initial decision should trigger expansion on root
    std::string action = planner.decide_best_action("Context", 1.0, 0.0);
    
    EXPECT_FALSE(action.empty());
    // Repeated calls should deepen the tree
    for(int i=0; i<5; ++i) {
        planner.decide_best_action("Context", 1.0, 0.0);
    }
}
