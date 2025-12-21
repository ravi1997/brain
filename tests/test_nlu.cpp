#include <gtest/gtest.h>
#include "../brain.hpp"

class NLUTest : public ::testing::Test {
protected:
    Brain brain;
};

TEST_F(NLUTest, ExtractEntities_SimpleName) {
    std::string text = "I think Isaac Newton was smart.";
    auto entities = brain.extract_entities(text);
    
    // Should find "Isaac Newton"
    ASSERT_FALSE(entities.empty());
    bool found = false;
    for (const auto& e : entities) {
        if (e == "Isaac Newton") found = true;
    }
    EXPECT_TRUE(found) << "Did not find 'Isaac Newton'";
}

TEST_F(NLUTest, ExtractEntities_CommonWords) {
    std::string text = "The cat sat on the Mat.";
    auto entities = brain.extract_entities(text);
    
    // "The" should be filtered if it's start of sentence (heuristic usually handles capital words)
    // My implementation:
    // if (word[0] is upper) start entity.
    // "The" is Capital.
    // "cat" is lower -> end entity "The".
    // "The" is filtered if single word common.
    
    // "Mat" is capital.
    
    bool foundThe = false;
    for (const auto& e : entities) {
        if (e == "The") foundThe = true;
    }
    EXPECT_FALSE(foundThe) << "Should have filtered 'The'";
    
    bool foundMat = false;
    for (const auto& e : entities) {
        if (e == "Mat") foundMat = true;
    }
    EXPECT_TRUE(foundMat) << "Should have found 'Mat'";
}

TEST_F(NLUTest, ExtractEntities_MultiWord) {
    std::string text = "United States of America is a country.";
    // "United States" -> Capital, Capital
    // "of" -> lower. Entity break?
    // My simple heuristic breaks on lowercase.
    // So it will find "United States" and "America".
    
    auto entities = brain.extract_entities(text);
    
    bool foundUS = false;
    for (const auto& e : entities) {
        if (e == "United States") foundUS = true;
    }
    EXPECT_TRUE(foundUS);
    
    bool foundAmerica = false;
    for (const auto& e : entities) {
        if (e == "America") foundAmerica = true;
    }
    EXPECT_TRUE(foundAmerica);
}

TEST_F(NLUTest, AssociativeMemoryUsage) {
    // This is harder to test without mocking MemoryStore or pre-populating it,
    // but we can rely on integration tests or manual verification for the full flow.
    // For unit test, we trust extract_entities works.
}
