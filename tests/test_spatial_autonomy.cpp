#include <gtest/gtest.h>
#include "../brain.hpp"
#include "../spatial_unit.hpp"
#include <cmath>

TEST(TestSpatial, LidarProcessing) {
    dnn::SpatialUnit spatial;
    
    // Simulate raw data: 8 sectors * 10 points each
    std::vector<unsigned char> raw_data(80);
    // Fill first sector with high values (obstacle)
    for(int i=0; i<10; ++i) raw_data[i] = 255;
    
    auto features = spatial.process_raw(raw_data);
    
    // Check features corresponding to first sector are high
    // 384 dim / 8 sectors = 48 features per sector
    ASSERT_EQ(features.size(), 384);
    
    double sector1_sum = 0.0;
    for(int i=0; i<48; ++i) sector1_sum += features[i];
    
    double sector2_sum = 0.0;
    for(int i=48; i<96; ++i) sector2_sum += features[i];
    
    std::cout << "Sector 1 Sum: " << sector1_sum << ", Sector 2 Sum: " << sector2_sum << std::endl;
    ASSERT_GT(sector1_sum, 1.0);
    ASSERT_LT(sector2_sum, 0.1); // Should be near zero
}

TEST(TestSpatial, NavigationState) {
    dnn::SpatialUnit spatial;
    spatial.update_position(1.0, 0.0, 0.0);
    spatial.update_position(0.0, 2.0, 0.5);
    
    auto coords = spatial.get_position();
    ASSERT_DOUBLE_EQ(coords.x, 1.0);
    ASSERT_DOUBLE_EQ(coords.y, 2.0);
    ASSERT_DOUBLE_EQ(coords.heading, 0.5);
}

TEST(TestAutonomy, GoalGeneration) {
    PlanningUnit planning;
    
    // Low Boredom -> No goals
    auto goals1 = planning.generate_goals(0.1, 0.5, "");
    ASSERT_TRUE(goals1.empty());
    
    // High Boredom + High Curiosity -> Research Goal
    auto goals2 = planning.generate_goals(0.9, 0.9, "physics");
    ASSERT_FALSE(goals2.empty());
    ASSERT_EQ(goals2[0].type, "RESEARCH");
    ASSERT_GT(goals2[0].priority, 80);
    ASSERT_EQ(goals2[0].description, "Research physics");
    
    // High Boredom + Low Curiosity -> Maintenance
    auto goals3 = planning.generate_goals(0.9, 0.1, "");
    ASSERT_FALSE(goals3.empty());
    ASSERT_EQ(goals3[0].type, "MAINTENANCE");
}

TEST(TestAutonomy, BrainIntegration) {
    // This is harder to test without mocking the whole loop,
    // but we can verify evaluate_goals *compiles* and runs if we expose it or trust the build.
    // For now, relying on unit tests for component logic.
}
