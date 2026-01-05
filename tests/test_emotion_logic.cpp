#include <gtest/gtest.h>
#include "brain.hpp"
#include <thread>
#include <vector>

// Mocking dependencies if necessary or using Brain directly
// Using Brain directly is integration testing.
class EmotionLogicTest : public ::testing::Test {
protected:
    Brain brain;
};

// Test 1: High Boredom -> Research
TEST_F(EmotionLogicTest, HighBoredomTriggersResearch) {
    // Set state
    brain.emotions.boredom = 1.0;
    brain.emotions.energy = 1.0;
    brain.personality.curiosity = 1.0;
    
    // Process goals
    brain.evaluate_goals();
    
    // Check if task added
    Task* t = brain.task_manager.get_next_task(); // Pop
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->type, TaskType::RESEARCH);
    // delete t; // Don't delete static/manage pointer
}

// Test 2: Low Energy -> Sleep
TEST_F(EmotionLogicTest, LowEnergyTriggersSleep) {
    brain.emotions.boredom = 0.5;
    brain.emotions.energy = 0.1; // Very tired
    
    brain.evaluate_goals();
    
    Task* t = brain.task_manager.get_next_task();
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->type, TaskType::SLEEP);
}

// Test 3: Mixed State (Competition)
TEST_F(EmotionLogicTest, MixedStatePrioritization) {
    // Boredom high, Energy med-low
    brain.emotions.boredom = 0.9;
    brain.emotions.energy = 0.4; 
    
    // Research score: (0.9*3) + (0.8*1.5) = 2.7 + 1.2 = 3.9
    // Energy < 0.3 check? No, 0.4 > 0.3.
    // Sleep score: (1.0 - 0.4) * 4.0 = 0.6 * 4.0 = 2.4
    // Research should win.
    
    brain.evaluate_goals();
    
    Task* t = brain.task_manager.get_next_task();
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->type, TaskType::RESEARCH);
}

// Test 4: Hunger -> Eat
TEST_F(EmotionLogicTest, HungerTriggersEat) {
    brain.metabolism.hunger = 0.9;
    brain.emotions.energy = 0.5;
    
    brain.evaluate_goals();
    
    Task* t = brain.task_manager.get_next_task();
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->type, TaskType::EAT);
}

// Test 5: Thirst -> Drink
TEST_F(EmotionLogicTest, ThirstTriggersDrink) {
    brain.metabolism.thirst = 0.9;
    brain.emotions.energy = 0.5;
    
    brain.evaluate_goals();
    
    Task* t = brain.task_manager.get_next_task();
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->type, TaskType::DRINK);
}

// Test 6: Metabolism Pulse
TEST_F(EmotionLogicTest, MetabolismPulse) {
    double initial_hunger = brain.metabolism.hunger;
    brain.environment.time_of_day = 12.0; // Day
    
    brain.metabolize_step();
    
    EXPECT_GT(brain.metabolism.hunger, initial_hunger);
}
