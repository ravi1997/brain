#include <gtest/gtest.h>
#include "../memory_store.hpp"
#include <filesystem>

class MemoryTest : public ::testing::Test {
protected:
    std::string test_db = "test_memory.db";
    
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

TEST_F(MemoryTest, InitAndStore) {
    MemoryStore store(test_db);
    ASSERT_TRUE(store.init());
    
    EXPECT_TRUE(store.store("Observation", "Test Content", "Tag1"));
    EXPECT_EQ(store.get_memory_count(), 1);
}

TEST_F(MemoryTest, Query) {
    MemoryStore store(test_db);
    store.init();
    store.store("Thought", "I like robots", "AI");
    store.store("Thought", "I like cats", "Animal");
    
    auto results = store.query("robots");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].content, "I like robots");
}
