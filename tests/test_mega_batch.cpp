#include <gtest/gtest.h>
#include "brain.hpp"
#include "tactile_unit.hpp"
#include "tool_registry.hpp"
#include "metacognition.hpp"
#include "federation.hpp"
#include "hal.hpp"
#include <cmath>

TEST(TestMegaBatch, Metacognition) {
    dnn::Metacognition meta;
    double initial_lr = meta.params.learning_rate;
    // Simulate failure
    meta.monitor_performance(0.1, 0.9);
    ASSERT_GT(meta.params.learning_rate, initial_lr); // Should increase plasticity
}

TEST(TestMegaBatch, TactileUnit) {
    dnn::TactileUnit tactile;
    std::vector<unsigned char> data(16, 0);
    
    // Normal touch
    data[0] = 100; // Pressure
    tactile.process_raw(data);
    ASSERT_FALSE(tactile.is_in_pain());
    
    // Painful touch
    data[0] = 250; // High pressure
    tactile.process_raw(data);
    ASSERT_TRUE(tactile.is_in_pain());
}

TEST(TestMegaBatch, ToolRegistry) {
    dnn::ToolRegistry registry;
    std::string res = registry.use_tool("CALCULATOR", "2+2");
    ASSERT_NE(res.find("RESULT"), std::string::npos);
    
    std::string err = registry.use_tool("NONEXISTENT", "");
    ASSERT_NE(err.find("ERROR"), std::string::npos);
}

TEST(TestMegaBatch, SwarmProtocol) {
    dnn::SwarmPacket packet;
    packet.sender_id = "BRAIN_1";
    packet.payload = "Hello";
    ASSERT_EQ(packet.sender_id, "BRAIN_1");
    // Mostly header verification for now
}

TEST(TestMegaBatch, Metabolism) {
    Brain brain;
    brain.emotions.energy = 1.0;
    brain.metabolize_step(); // Should reduce energy
    ASSERT_LT(brain.emotions.energy, 1.0);
}

TEST(TestMegaBatch, TheoryOfMind) {
    Brain brain;
    brain.update_context("User", "I am very happy with this!", "Statement");
    ASSERT_GT(brain.user_model.estimated_happiness, 0.5); // Should increase
}

TEST(TestMegaBatch, Conditioning) {
    Brain brain;
    // Standard: Trigger -> Reward
    brain.update_context("User", "Bell", "Trigger");
    brain.update_context("User", "Good job!", "Reward"); // High Sentiment
    
    ASSERT_TRUE(brain.condition_map.count("bell"));
    ASSERT_EQ(brain.condition_map["bell"], "POSITIVE_RESPONSE");
}

TEST(TestMegaBatch, Federation) {
    dnn::FederationUnit fed;
    fed.propose_fact({"Sky", "is", "Blue", 0.9, "User1"});
    auto accepted = fed.sync_knowledge();
    ASSERT_EQ(accepted.size(), 1);
    ASSERT_EQ(accepted[0].subject, "Sky");
}

TEST(TestMegaBatch, HardwareAbstraction) {
    dnn::CpuAccelerator cpu;
    std::vector<double> vec = {1.0, 2.0};
    std::vector<std::vector<double>> batch = {{1.0, 1.0}, {2.0, 2.0}}; // {3, 6}
    std::vector<double> results(2);
    
    cpu.dot_product_batch(vec, batch, results);
    ASSERT_DOUBLE_EQ(results[0], 3.0);
    ASSERT_DOUBLE_EQ(results[1], 6.0);
}

// Note: Testing Postgres ACLs requires live DB; stubbing logic or using integration tests usually better
// But we modified MemoryStore logic so let's check basic struct updates (can't test SQL execution purely here without mock)
