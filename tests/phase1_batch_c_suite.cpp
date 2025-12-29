#include <gtest/gtest.h>
#include "../include/perception/scene_graph.hpp"
#include "../include/optimization/aco.hpp"
#include "../include/neural/uncertainty.hpp"
#include "../include/nlu/relation_extraction.hpp"
#include "../include/infra/kb_completion.hpp"

// Scene Graph Generation Test
TEST(Phase1_Perception, SceneGraphGeneration) {
    dnn::perception::SceneGraphGenerator sgg;
    
    std::vector<dnn::perception::SceneGraphGenerator::Object> objects = {
        {"person", 0, 0.9f, {100, 100, 50, 100}},
        {"car", 1, 0.8f, {200, 200, 80, 60}}
    };
    
    auto graph = sgg.generate(objects);
    ASSERT_EQ(graph.objects.size(), 2);
    ASSERT_GT(graph.relationships.size(), 0);
    
    auto cars = sgg.query_objects(graph, "car");
    ASSERT_EQ(cars.size(), 1);
}

// Ant Colony Optimization Test
TEST(Phase1_Optimization, AntColonyOptimization) {
    dnn::optimization::AntColonyOptimization aco(20, 5);  //20 ants, 5 nodes
    
    // Simple cost: sum of path
    auto cost_fn = [](const std::vector<int>& path) {
        float cost = 0.0f;
        for (int node : path) cost += node;
        return cost;
    };
    
    auto solution = aco.optimize(cost_fn, 10);  // 10 iterations
    ASSERT_EQ(solution.size(), 5);
}

// Uncertainty Quantification Test
TEST(Phase1_Neural, UncertaintyQuantification) {
    dnn::neural::UncertaintyQuantification uq(0.1f);
    
    std::vector<float> activations = {1.0f, 2.0f, 3.0f};
    auto dropped = uq.apply_dropout(activations, true);
    ASSERT_EQ(dropped.size(), 3);
    
    // Test uncertainty estimation
    auto forward_fn = [&uq](const std::vector<float>& input, bool training) {
        return uq.apply_dropout(input, training);
    };
    
    auto estimate = uq.estimate(forward_fn, activations, 10);
    ASSERT_EQ(estimate.mean.size(), 3);
    ASSERT_EQ(estimate.epistemic_uncertainty.size(), 3);
}

// Relation Extraction Test
TEST(Phase1_NLU, RelationExtraction) {
    dnn::nlu::RelationExtraction re;
    
    std::string text = "Paris is the capital of France. John works for Google.";
    auto triples = re.extract(text);
    
    ASSERT_GT(triples.size(), 0);
    
    bool found_capital = false;
    for (const auto& triple : triples) {
        if (triple.relation == "capital of") {
            found_capital = true;
            break;
        }
    }
    ASSERT_TRUE(found_capital);
}

// Knowledge Base Completion Test
TEST(Phase1_Infra, KnowledgeBaseCompletion) {
    dnn::infra::KnowledgeBaseCompletion kbc;
    
    // Add facts
    kbc.add_fact({"Cat", "subclass_of", "Mammal"});
    kbc.add_fact({"Mammal", "subclass_of", "Animal"});
    
    // Complete KB (should infer Cat subclass_of Animal via transitivity)
    auto inferred = kbc.complete();
    ASSERT_GT(inferred.size(), 0);
    
    // Query
    auto results = kbc.query("Cat", "subclass_of", "Animal");
    ASSERT_GT(results.size(), 0);
}
