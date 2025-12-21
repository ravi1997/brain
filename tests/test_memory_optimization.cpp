#include <gtest/gtest.h>
#include "../memory_store.hpp"
#include <filesystem>

class MemoryOptimizationTest : public ::testing::Test {
protected:
    std::string test_db = "test_memory_opt.db";
    
    void SetUp() override {
        if (std::filesystem::exists(test_db)) {
            std::filesystem::remove(test_db);
        }
    }

    void TearDown() override {
        if (std::filesystem::exists(test_db)) {
             std::filesystem::remove(test_db);
        }
    }
};

TEST_F(MemoryOptimizationTest, ExactTokenMatch) {
    MemoryStore store(test_db);
    store.init();
    store.store("Fact", "The sky is blue", "nature");
    
    // Should match "blue"
    auto results = store.query("blue");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].content, "The sky is blue");
    
    // Should NOT match "blu" (substring) - unlike SQL LIKE
    results = store.query("blu");
    EXPECT_EQ(results.size(), 0);
}

TEST_F(MemoryOptimizationTest, ShortWordExclusion) {
    MemoryStore store(test_db);
    store.init();
    store.store("Fact", "It is an AI", "tech");
    
    // "is", "an", "AI" are < 3 chars (or 2 chars). 
    // Logic: length < 3 continue. So length 2 is skipped.
    
    auto results = store.query("is");
    EXPECT_EQ(results.size(), 0);
    
    results = store.query("AI"); // Length 2
    EXPECT_EQ(results.size(), 0);
}

TEST_F(MemoryOptimizationTest, CaseInsensitivity) {
    MemoryStore store(test_db);
    store.init();
    store.store("Fact", "Robots are COOL", "tech");
    
    auto results = store.query("robots");
    ASSERT_EQ(results.size(), 1);
    
    results = store.query("cool");
    ASSERT_EQ(results.size(), 1);
}
