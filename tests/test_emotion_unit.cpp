#include <gtest/gtest.h>
#include "brain.hpp"

class EmotionTest : public ::testing::Test {
protected:
    Brain brain;
};

TEST_F(EmotionTest, DecayHappiness) {
    brain.emotions.happiness = 1.0;
    // Simulate decay over many cycles if we could call automata_loop's logic, 
    // but for unit test, we check if analyze_sentiment affects it.
    
    double s = brain.analyze_sentiment("I hate this terrible world.");
    if (s < 0) brain.emotions.sadness = std::min(1.0, brain.emotions.sadness + 0.1);
    
    EXPECT_GT(brain.emotions.sadness, 0.0);
}

TEST_F(EmotionTest, BoredomReactivity) {
    brain.emotions.boredom = 0.9;
    // In automata_loop, boredom decreases on interaction
    brain.interact("Hello there!");
    EXPECT_LT(brain.emotions.boredom, 0.9);
}

TEST_F(EmotionTest, EnergyCriticalState) {
    brain.emotions.energy = 0.1;
    std::string resp = brain.interact("Wake up!");
    // Should give a sleepy response
    bool sleepy = (resp.find("sleep") != std::string::npos || resp.find("tired") != std::string::npos || resp.find("Energy levels critical") != std::string::npos);
    EXPECT_TRUE(sleepy);
}
