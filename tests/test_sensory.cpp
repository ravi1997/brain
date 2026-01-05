#include <gtest/gtest.h>
#include "brain.hpp"
#include "vision_unit.hpp"
#include <memory>

class SensoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        brain = std::make_unique<Brain>();
    }

    std::unique_ptr<Brain> brain;
};

TEST_F(SensoryTest, RegisterSensoryUnitManual) {
    auto vision = std::make_unique<dnn::VisionUnit>(std::vector<std::size_t>{10, 5, 384});
    brain->register_sensory_unit(std::move(vision));
    std::string state = brain->get_json_state();
    EXPECT_NE(state.find("Ocular Interface"), std::string::npos);
    EXPECT_NE(state.find("sensory_activity"), std::string::npos);
}

TEST_F(SensoryTest, SensoryFocusDecay) {
    // There is one unit registered via Brain constructor
    brain->update_sensory_focus(); 
    std::vector<double> aggregate = brain->get_aggregate_sensory_input();
    EXPECT_EQ(aggregate.size(), 384);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
