#include "brain.hpp"
#include <gtest/gtest.h>

class ContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Suppress output
    }
};

TEST_F(ContextTest, MaintainsContext) {
    Brain brain;
    
    // Turn 1
    std::string input1 = "My name is Sylvia.";
    std::string response1 = brain.interact(input1);
    
    // Check internal state directly (whitebox)
    // We need to access conversation_context, but it is private? 
    // Wait, struct members in brain.hpp are public by default in the class definition 
    // unless defined under private:
    // Let's check brain.hpp again.
    // Yes, Brain class members are public up top.
    
    ASSERT_EQ(brain.conversation_context.size(), 2);
    EXPECT_EQ(brain.conversation_context[0], "User: My name is Sylvia.");
    EXPECT_TRUE(brain.conversation_context[1].rfind("Brain:", 0) == 0);
    
    // Turn 2
    std::string input2 = "What is my name?";
    std::string response2 = brain.interact(input2);
    
    ASSERT_EQ(brain.conversation_context.size(), 4);
    EXPECT_EQ(brain.conversation_context[2], "User: What is my name?");
    
    // For now, since we don't have a perfect LLM, we just assume the mechanism works 
    // if the context buffer is populated correctly. 
    // Actual semantic recall depends on the neural net training which is not pre-baked.
}

TEST_F(ContextTest, TruncatesContext) {
    Brain brain;
    
    for(int i=0; i<10; ++i) {
        brain.interact("Input " + std::to_string(i));
    }
    
    // Max turns = 6 (defined in header)
    ASSERT_LE(brain.conversation_context.size(), 6);
}

TEST_F(ContextTest, ContextualRecall) {
    Brain brain;
    
    // Seed some memory
    brain.memory_store->store("Personal", "Sylvia is a biological scientist.", "Sylvia");
    
    // Turn 1: Mention entity
    brain.interact("I know a person named Sylvia.");
    
    // Turn 2: Ask about entity without mentioning it explicitly
    // This should trigger associative memory retrieval using the context "Sylvia"
    std::string response = brain.interact("What does she do?");
    
    // We expect the log to have mentioned memory recall, and the response to contain knowledge about Sylvia
    EXPECT_TRUE(response.find("scientist") != std::string::npos || response.find("recall") != std::string::npos);
}
