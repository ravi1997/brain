#include <gtest/gtest.h>
#include "brain.hpp"
#include <memory>

/**
 * Detailed Test Suite for Biological Dynamics
 * Verifies metabolism, hormonal responses, and goal prioritization.
 */

class BiologicalDynamicsTest : public ::testing::Test {
protected:
    std::unique_ptr<Brain> brain;

    void SetUp() override {
        brain = std::make_unique<Brain>();
        // Reset to a clean known state for consistent testing
        std::lock_guard<std::recursive_mutex> lock(brain->brain_mutex);
        brain->metabolism.hunger = 0.0;
        brain->metabolism.thirst = 0.0;
        brain->metabolism.glucose = 1.0;
        brain->emotions.energy = 1.0;
        brain->hormones.cortisol = 0.0;
        brain->hormones.melatonin = 0.0;
        brain->environment.time_of_day = 12.0; // Noon
        brain->environment.is_daylight = true;
        
        // Ensure no pending tasks from initialization
        while(brain->task_manager.has_pending_tasks()) {
            brain->task_manager.get_next_task();
            brain->task_manager.complete_active_task();
        }
    }
};

// Test 1: Metabolism Decay over a single step
TEST_F(BiologicalDynamicsTest, Metabolism_SingleStep_Decay) {
    double initial_hunger = brain->metabolism.hunger;
    double initial_thirst = brain->metabolism.thirst;
    double initial_glucose = brain->metabolism.glucose;
    double initial_energy = brain->emotions.energy;

    brain->metabolize_step();

    EXPECT_GT(brain->metabolism.hunger, initial_hunger);
    EXPECT_GT(brain->metabolism.thirst, initial_thirst);
    EXPECT_LT(brain->metabolism.glucose, initial_glucose);
    EXPECT_LT(brain->emotions.energy, initial_energy);
}

// Test 2: Stress response (Cortisol) to high hunger
TEST_F(BiologicalDynamicsTest, Hormonal_StressResponse_Hunger) {
    brain->metabolism.hunger = 0.8; // Trigger threshold is 0.7
    brain->hormones.cortisol = 0.0;

    brain->metabolize_step();

    EXPECT_GT(brain->hormones.cortisol, 0.0) << "Cortisol should increase when hunger > 0.7";
}

// Test 3: Day/Night Cycle and Melatonin
TEST_F(BiologicalDynamicsTest, DayNight_Melatonin_Production) {
    // Set to night time
    brain->environment.time_of_day = 20.0; // 8 PM
    brain->metabolize_step(); // This will update environment.is_daylight

    EXPECT_FALSE(brain->environment.is_daylight);
    
    double initial_melatonin = brain->hormones.melatonin;
    brain->metabolize_step();
    EXPECT_GT(brain->hormones.melatonin, initial_melatonin) << "Melatonin should increase at night";

    // Morning transition
    brain->environment.time_of_day = 8.0; // 8 AM
    brain->metabolize_step();
    EXPECT_TRUE(brain->environment.is_daylight);
    
    initial_melatonin = brain->hormones.melatonin;
    brain->metabolize_step();
    EXPECT_LT(brain->hormones.melatonin, initial_melatonin) << "Melatonin should decrease in daylight";
}

// Test 4: Goal Prioritization - Hunger leads to Eating
TEST_F(BiologicalDynamicsTest, GoalPriority_Hunger_Triggers_EatTask) {
    brain->metabolism.hunger = 0.95;
    brain->metabolism.thirst = 0.1;
    brain->emotions.energy = 0.8;
    
    brain->evaluate_goals();
    
    Task* t = brain->task_manager.get_next_task();
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->type, TaskType::EAT);
}

// Test 5: Goal Prioritization - Thirst overrides Hunger when higher
TEST_F(BiologicalDynamicsTest, GoalPriority_Thirst_Overrides_Hunger) {
    brain->metabolism.hunger = 0.5;
    brain->metabolism.thirst = 0.9;
    brain->emotions.energy = 0.8;
    
    brain->evaluate_goals();
    
    Task* t = brain->task_manager.get_next_task();
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->type, TaskType::DRINK);
}

// Test 6: Low glucose causes energy drop
TEST_F(BiologicalDynamicsTest, Glucose_Starvation_EnergyDrop) {
    brain->metabolism.glucose = 0.1; // Below 0.3 threshold
    double initial_energy = brain->emotions.energy;

    brain->metabolize_step();

    EXPECT_LT(brain->emotions.energy, initial_energy - 0.001) << "Energy should drop faster when glucose is low";
}

// Test 7: Circadian Rhythm - High melatonin causes sleepiness
TEST_F(BiologicalDynamicsTest, Melatonin_Causes_Sleepiness) {
    brain->hormones.melatonin = 0.8; // High melatonin
    double initial_energy = brain->emotions.energy;

    brain->metabolize_step();

    EXPECT_LT(brain->emotions.energy, initial_energy - 0.01) << "High melatonin should drain energy";
}
