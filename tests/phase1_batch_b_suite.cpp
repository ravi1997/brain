#include <gtest/gtest.h>
#include "../include/neural/continual_learning.hpp"
#include "../include/neural/bayesian_layer.hpp"
#include "../include/neural/td_learning.hpp"
#include "../include/reasoning/analogical_reasoning.hpp"
#include "../include/reasoning/inference_engine.hpp"

// Continual Learning (EWC) Test
TEST(Phase1_Neural, ContinualLearning) {
    dnn::neural::ContinualLearning cl(100);
    
    // Simulate gradients
    std::vector<std::vector<float>> gradients = {
        std::vector<float>(100, 0.1f),
        std::vector<float>(100, 0.2f)
    };
    
    cl.compute_fisher(gradients);
    cl.consolidate();
    
    std::vector<float> gradient(100, 0.05f);
    cl.update(gradient);
    
    auto weights = cl.get_weights();
    ASSERT_EQ(weights.size(), 100);
}

// Bayesian Layer Test
TEST(Phase1_Neural, BayesianLayer) {
    dnn::neural::BayesianLayer layer(10,  5);
    
    std::vector<float> input(10, 1.0f);
    auto output = layer.forward(input, false);  // Deterministic
    ASSERT_EQ(output.size(), 5);
    
    auto [mean, var] = layer.forward_with_uncertainty(input, 10);
    ASSERT_EQ(mean.size(), 5);
    ASSERT_EQ(var.size(), 5);
}

// Temporal Difference Learning Test
TEST(Phase1_Neural, TDLearning) {
    dnn::neural::TemporalDifferenceLearning td(10, 4);  // 10 states, 4 actions
    
    int action = td.select_action(0, 0.1f);
    ASSERT_GE(action, 0);
    ASSERT_LT(action, 4);
    
    td.update(0, action, 1.0f, 1, 0);  // Reward=1.0
    
    float q = td.get_q(0, action);
    ASSERT_TRUE(q != 0.0f);  // Should have updated
}

// Analogical Reasoning Test
TEST(Phase1_Reasoning, AnalogicalReasoning) {
    dnn::reasoning::AnalogicalReasoning ar;
    
    std::vector<dnn::reasoning::AnalogicalReasoning::Relation> source = {
        {"parent_of", "John", "Mary"}
    };
    
    std::vector<dnn::reasoning::AnalogicalReasoning::Relation> target = {
        {"parent_of", "Alice", "Bob"}
    };
    
    auto analogy = ar.find_analogy(source, target);
    ASSERT_GT(analogy.similarity_score, 0.0f);
    ASSERT_FALSE(analogy.mappings.empty());
}

// Inference Engine Test
TEST(Phase1_Reasoning, InferenceEngine) {
    dnn::reasoning::InferenceEngine ie;
    
    // Add facts
    ie.add_fact("Socrates is a man");
    
    // Add rule: All men are mortal
    ie.add_rule({{"Socrates is a man"}, "Socrates is mortal"});
    
    // Forward chaining
    auto new_facts = ie.forward_chain();
    auto facts = ie.get_facts();
    ASSERT_TRUE(facts.count("Socrates is mortal"));
    
    // Backward chaining
    bool proven = ie.prove("Socrates is mortal");
    ASSERT_TRUE(proven);
}
