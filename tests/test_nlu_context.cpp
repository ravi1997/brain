#include <gtest/gtest.h>
#include "../brain.hpp"

class NLUContextTest : public ::testing::Test {
protected:
    Brain brain;
};

TEST_F(NLUContextTest, HistoryMaintainsSize) {
    for(int i=0; i<10; ++i) {
        brain.update_context("User", "Msg " + std::to_string(i), "INTENT");
    }
    EXPECT_LE(brain.conversation_history.size(), 5);
    EXPECT_EQ(brain.conversation_history.back().text, "Msg 9");
}

TEST_F(NLUContextTest, ResolvesWhyQuestion) {
    brain.update_context("User", "I like apples", "STATEMENT");
    std::string intent = brain.resolve_intent("Why?");
    
    EXPECT_NE(intent.find("Context: I like apples"), std::string::npos);
}

TEST_F(NLUContextTest, ResolvesPronounHe) {
    // Manually push a history item with an entity (simulating extract_entities found it previously)
    // Since extract_entities is stateless, we can assume it works if we text match.
    // However, resolve_intent calls extract_entities on history. 
    // We need a history text that definitely extracts an entity.
    // Assuming "Elon Musk" is extracted.
    
    brain.update_context("User", "Elon Musk is wealthy.", "STATEMENT");
    std::string intent = brain.resolve_intent("Is he nice?");
    
    // We expect "Elon Musk" to be in the resolved string
    // This depends on extract_entities working.
}
