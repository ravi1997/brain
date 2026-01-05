#include <gtest/gtest.h>
#include "brain.hpp"
#include <algorithm>

class ImprovedNLUTest : public ::testing::Test {
protected:
    Brain brain;
};

TEST_F(ImprovedNLUTest, SentenceStartFiltersCommonWords) {
    // "Actually" is capitalized at start, but should be filtered if it's a stop-word or too common.
    // In my impl, if it's length > 3 and not a stop word, it might still pass, but let's test a known stop word.
    std::string text = "But I think the Eiffel Tower is great.";
    auto entities = brain.extract_entities(text);
    
    auto it = std::find(entities.begin(), entities.end(), "But");
    EXPECT_EQ(it, entities.end()) << "Should have filtered 'But' at sentence start";
    
    it = std::find(entities.begin(), entities.end(), "Eiffel Tower");
    EXPECT_NE(it, entities.end()) << "Should have found 'Eiffel Tower'";
}

TEST_F(ImprovedNLUTest, AcronymExtraction) {
    std::string text = "the NASA agency studies space.";
    auto entities = brain.extract_entities(text);
    
    auto it = std::find(entities.begin(), entities.end(), "NASA");
    EXPECT_NE(it, entities.end()) << "Should have found acronym 'NASA'";
}

TEST_F(ImprovedNLUTest, SentenceStartLegitimateEntity) {
    // "Brain" at start of sentence. "Brain" might be a stop word? Let's check.
    // Actually, "Brain" is 5 chars, so it should pass if not a stop word.
    std::string text = "Mercury is the smallest planet.";
    auto entities = brain.extract_entities(text);
    
    auto it = std::find(entities.begin(), entities.end(), "Mercury");
    EXPECT_NE(it, entities.end()) << "Should have found 'Mercury' even at sentence start";
}

TEST_F(ImprovedNLUTest, MiddleOfSentence) {
    std::string text = "I am using the Brain AI.";
    auto entities = brain.extract_entities(text);
    
    auto it = std::find(entities.begin(), entities.end(), "Brain AI");
    EXPECT_NE(it, entities.end()) << "Should have joined Brain and AI";
}
