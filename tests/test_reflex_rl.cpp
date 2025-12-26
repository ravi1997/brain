#include <gtest/gtest.h>
#include "../reflex.hpp"

class ReflexRLTest : public ::testing::Test {
protected:
    Reflex reflex;
};

TEST_F(ReflexRLTest, WeightedResponseSelection) {
    // hello has two responses by default: "Greetings." and "Hello there."
    // We'll reinforce one heavily and check if it's picked more often
    
    for (int i = 0; i < 50; ++i) {
        reflex.reinforce("hello", "Greetings.", 1.0);
    }
    
    int greetings_count = 0;
    for (int i = 0; i < 100; ++i) {
        if (reflex.get_reaction("hello") == "Greetings.") {
            greetings_count++;
        }
    }
    
    // With heavy reinforcement, it should definitely be > 80%
    EXPECT_GT(greetings_count, 80);
}

TEST_F(ReflexRLTest, NegativeReinforcement) {
    // Status has "SYSTEM ONLINE." and "Neural pathways active."
    // We'll discourage "SYSTEM ONLINE."
    
    for (int i = 0; i < 50; ++i) {
        reflex.reinforce("status", "SYSTEM ONLINE.", -1.0);
    }
    
    int online_count = 0;
    for (int i = 0; i < 100; ++i) {
        if (reflex.get_reaction("status") == "SYSTEM ONLINE.") {
            online_count++;
        }
    }
    
    // Should be very rare now (clamped at 0.1 weight)
    EXPECT_LT(online_count, 20);
}
