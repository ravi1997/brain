#include <gtest/gtest.h>
#include "../reflex.hpp"
#include <iostream>

class ReflexFuzzyTest : public ::testing::Test {
protected:
    Reflex reflex;
};

// Test 1: Typos (Levenshtein Distance)
TEST_F(ReflexFuzzyTest, TypoDetection) {
    // "hello" is a standard instinct
    // "helo" (missing 'l') should trigger it via fuzzy match (diff=1)
    std::string reaction = reflex.get_reaction("helo");
    bool match = (reaction == "Greetings." || reaction == "Hello there.");
    EXPECT_TRUE(match) << "Failed to fuzzy match 'helo' to 'hello'. Got: " << reaction;
    
    // "hlo" (missing 'e', 'l') -> diff=2 (might fail depending on threshold)
    // Our threshold is <= 1 for > 3 char words
}

TEST_F(ReflexFuzzyTest, NoFalsePositives) {
    // "hell" -> matches "hello" by 1 char diff?
    // "hello" length is 5. "hell" length is 4. diff is 1. match logic says <=1 and len > 3. so it triggers.
    // This is debatable if false positive or acceptable.
    // Let's check something distant. "hero" -> "hello" (diff: r!=l, l missing? no len same.)
    // hero vs hello: h(match) e(match) r!=l l!=l o(match). diff = 2. Should NOT match.
    
    std::string reaction = reflex.get_reaction("hero");
    EXPECT_EQ(reaction, "") << "False positive: 'hero' matched 'hello'";
}
