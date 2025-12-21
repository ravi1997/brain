#include "../brain.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <string>

// Test that temporal entities are extracted
TEST(ImprovedNLUTest, ExtractTimeEntities) {
    Brain brain;
    
    std::string input = "Meet me at 5pm tomorrow";
    auto entities = brain.extract_entities(input);
    
    // Should extract "5pm" and "tomorrow"
    EXPECT_GE(entities.size(), 2);
    
    bool found_5pm = false;
    bool found_tomorrow = false;
    for (const auto& e : entities) {
        if (e == "5pm") found_5pm = true;
        if (e == "tomorrow") found_tomorrow = true;
    }
    
    EXPECT_TRUE(found_5pm);
    EXPECT_TRUE(found_tomorrow);
}

// Test that stopwords are filtered out
TEST(ImprovedNLUTest, FilterStopwords) {
    Brain brain;
    
    std::string input = "The quick brown fox jumps";
    auto entities = brain.extract_entities(input);
    
    // "The" should be filtered out as a stopword
    for (const auto& e : entities) {
        EXPECT_NE(e, "The");
        EXPECT_NE(e, "the");
    }
}

// Test multi-word entity extraction
TEST(ImprovedNLUTest, ExtractMultiWordEntities) {
    Brain brain;
    
    std::string input = "I love New York City and San Francisco";
    auto entities = brain.extract_entities(input);
    
    // Should extract "New York City" or "San Francisco"
    bool found_nyc = false;
    bool found_sf = false;
    
    for (const auto& e : entities) {
        if (e.find("New York City") != std::string::npos) found_nyc = true;
        if (e.find("San Francisco") != std::string::npos) found_sf = true;
    }
    
    EXPECT_TRUE(found_nyc || found_sf);
}

// Test time pattern extraction
TEST(ImprovedNLUTest, ExtractTimePatterns) {
    Brain brain;
    
    std::string input = "The meeting is at 10:30am";
    auto entities = brain.extract_entities(input);
    
    bool found_time = false;
    for (const auto& e : entities) {
        if (e == "10:30am") found_time = true;
    }
    
    EXPECT_TRUE(found_time);
}

// Test that proper nouns are still extracted
TEST(ImprovedNLUTest, ExtractProperNouns) {
    Brain brain;
    
    std::string input = "I visited Paris last week";
    auto entities = brain.extract_entities(input);
    
    bool found_paris = false;
    for (const auto& e : entities) {
        if (e == "Paris") found_paris = true;
    }
    
    EXPECT_TRUE(found_paris);
}
