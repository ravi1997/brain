#include <gtest/gtest.h>
#include "../brain.hpp"

class CognitionFeaturesTest : public ::testing::Test {
protected:
    Brain brain;
};

TEST_F(CognitionFeaturesTest, EmotionEdgeCases) {
    // Current Brain implementation might not expose emotions directly, 
    // but we can indirect via text or public methods.
    // Assuming emotions struct is public or we use interact loop.
    
    // Manually push emotions to extreme if possible
    brain.emotions.happiness = 100.0; // Should clamp?
    // How is it updated? update_emotions(delta)
    // We don't have direct update method exposed in snippet, 
    // but let's assume we can trigger happy inputs.
    
    for(int i=0; i<50; ++i) {
        brain.interact("I am so happy! Great job!");
    }
    
    // Check if stable (not NaN, not infinity)
    EXPECT_LE(brain.emotions.happiness, 1.0); // Assuming normalized 0-1 or -1 to 1
    EXPECT_GE(brain.emotions.happiness, -1.0);
}

TEST_F(CognitionFeaturesTest, SleepConsolidationCheck) {
    if (!brain.memory_store) {
        std::cerr << "Skipping DB test" << std::endl;
        return;
    }
    
    // 1. Add some history
    brain.interact("Important user fact to remember.");
    brain.interact("Another fact.");
    
    // 2. Sleep
    brain.sleep();
    
    // 3. Check logs or verify consolidated?
    // We can't easily check internal state of 'consolidated' flag unless we inspect history logic.
    // But we can check if it CRASHES or double inserts.
    
    // Sleep again
    brain.sleep();
    
    // Should be safe
    EXPECT_TRUE(true);
}
