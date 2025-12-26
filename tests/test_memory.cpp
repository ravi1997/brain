#include <gtest/gtest.h>
#include "../memory_store.hpp"
#include <filesystem>

class MemoryTest : public ::testing::Test {
protected:
    std::string test_conn = "host=postgres dbname=brain_db user=brain_user password=brain_password";
    
    void SetUp() override {
        MemoryStore store(test_conn);
        store.init();
        store.clear();
    }

    void TearDown() override {
        // Could clear tables here, but for now we'll just let it persist
    }
};

TEST_F(MemoryTest, InitAndStore) {
    MemoryStore store(test_conn);
    ASSERT_TRUE(store.init());
    
    EXPECT_TRUE(store.store("Observation", "Test Content", "Tag1"));
    // Count might be > 1 if tests are rerun without table wipe
    EXPECT_GE(store.get_memory_count(), 1);
}

TEST_F(MemoryTest, Query) {
    MemoryStore store(test_conn);
    store.init();
    store.store("Thought", "I like robots", "AI");
    store.store("Thought", "I like cats", "Animal");
    
    auto results = store.query("robots");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].content, "I like robots");
}
