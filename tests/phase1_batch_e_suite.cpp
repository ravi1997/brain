#include <gtest/gtest.h>
#include "neural/neural_rendering.hpp"
#include "neural/attention_flow.hpp"
#include "reasoning/temporal_logic.hpp"
#include "reasoning/htn_planner.hpp"
#include "perception/vqa.hpp"

// Neural Rendering Test
TEST(Phase1_Neural, NeuralRendering) {
    dnn::neural::NeuralRendering nerf(4, 64);  // Small network for testing
    
    // Test point query
    std::vector<float> position = {0.5f, 0.5f, 0.5f};
    std::vector<float> direction = {0.0f, 0.0f, 1.0f};
    
    auto sample = nerf.query_point(position, direction);
    ASSERT_GE(sample.r, 0.0f);
    ASSERT_LE(sample.r, 1.0f);
    ASSERT_GE(sample.density, 0.0f);
    
    // Test ray rendering
    dnn::neural::NeuralRendering::Ray ray;
    ray.origin = {0.0f, 0.0f, 0.0f};
    ray.direction = {0.0f, 0.0f, 1.0f};
    
    auto color = nerf.render_ray(ray, 16);  // Few samples for speed
    ASSERT_EQ(color.size(), 3);
}

// Attention Flow Visualization Test
TEST(Phase1_Neural, AttentionFlowVisualization) {
    dnn::neural::AttentionFlowVisualization afv;
    
    // Create mock attention head
    dnn::neural::AttentionFlowVisualization::AttentionHead head;
    head.head_id = 0;
    head.num_queries = 4;
    head.num_keys = 4;
    
    // Mock attention weights (4x4)
    head.attention_weights = {
        {0.7f, 0.1f, 0.1f, 0.1f},
        {0.1f, 0.7f, 0.1f, 0.1f},
        {0.1f, 0.1f, 0.7f, 0.1f},
        {0.1f, 0.1f, 0.1f, 0.7f}
    };
    
    afv.add_layer(0, {head});
    
    auto flow = afv.compute_attention_flow();
    ASSERT_EQ(flow.size(), 4);
    
    auto patterns = afv.detect_patterns(0.05f);
    ASSERT_GT(patterns.size(), 0);
    
    auto important = afv.get_important_tokens(2);
    ASSERT_LE(important.size(), 2);
}

// Temporal Logic Reasoning Test
TEST(Phase1_Reasoning, TemporalLogicReasoning) {
    dnn::reasoning::TemporalLogicReasoning tlr;
    
    // Create trace: state sequence
    std::vector<std::unordered_set<std::string>> seq = {
        {"p"},
        {"p", "q"},
        {"q"},
        {"r"}
    };
    
    auto trace = tlr.create_trace(seq);
    ASSERT_EQ(trace.size(), 4);
    
    // Test EVENTUALLY: F q (q eventually holds)
    auto q = dnn::reasoning::TemporalLogicReasoning::TemporalFormula::atom("q");
    auto eventually_q = dnn::reasoning::TemporalLogicReasoning::TemporalFormula::eventually(q);
    ASSERT_TRUE(tlr.check_formula(eventually_q, trace, 0));
    
    // Test ALWAYS: G p (p always holds) - should be false
    auto p = dnn::reasoning::TemporalLogicReasoning::TemporalFormula::atom("p");
    auto always_p = dnn::reasoning::TemporalLogicReasoning::TemporalFormula::always(p);
    ASSERT_FALSE(tlr.check_formula(always_p, trace, 0));
    
    // Test safety
    ASSERT_TRUE(tlr.check_safety("bad", trace));
    
    // Test liveness
    ASSERT_TRUE(tlr.check_liveness("r", trace));
}

// HTN Planner Test
TEST(Phase1_Reasoning, HTNPlanner) {
    dnn::reasoning::HTNPlanner planner;
    
    // Define primitive actions
    dnn::reasoning::HTNPlanner::Action pickup("pickup");
    pickup.preconditions = {"at_location", "clear"};
    pickup.add_effects = {"holding"};
    pickup.delete_effects = {"clear"};
    planner.add_action(pickup);
    
    dnn::reasoning::HTNPlanner::Action putdown("putdown");
    putdown.preconditions = {"holding"};
    putdown.add_effects = {"clear"};
    putdown.delete_effects = {"holding"};
    planner.add_action(putdown);
    
    // Define method for compound task
    dnn::reasoning::HTNPlanner::Method transport_method("transport", "transport_object");
    transport_method.preconditions = {"at_location"};
    transport_method.subtasks = {
        dnn::reasoning::HTNPlanner::Task("pickup", true),
        dnn::reasoning::HTNPlanner::Task("putdown", true)
    };
    planner.add_method(transport_method);
    
    // Initial state
    dnn::reasoning::HTNPlanner::State initial;
    initial.facts = {"at_location", "clear"};
    
    // Goal
    std::vector<dnn::reasoning::HTNPlanner::Task> goals = {
        dnn::reasoning::HTNPlanner::Task("transport_object", false)
    };
    
    auto plan = planner.plan(goals, initial);
    ASSERT_GT(plan.size(), 0);
}

// Visual Question Answering Test
TEST(Phase1_Perception, VisualQuestionAnswering) {
    dnn::perception::VisualQuestionAnswering vqa;
    
    // Create visual features
    std::vector<dnn::perception::VisualQuestionAnswering::VisualFeature> features;
    
    dnn::perception::VisualQuestionAnswering::VisualFeature person;
    person.object_name = "person";
    person.bbox = {0.3f, 0.4f, 0.2f, 0.4f};
    person.confidence = 0.9f;
    features.push_back(person);
    
    dnn::perception::VisualQuestionAnswering::VisualFeature car;
    car.object_name = "car";
    car.bbox = {0.6f, 0.7f, 0.3f, 0.2f};
    car.confidence = 0.85f;
    features.push_back(car);
    
    // Test "what" question
    dnn::perception::VisualQuestionAnswering::Question q1("What is in the image?");
    auto answer1 = vqa.answer(q1, features);
    ASSERT_FALSE(answer1.empty());
    
    // Test "count" question
    dnn::perception::VisualQuestionAnswering::Question q2("How many objects are there?");
    auto answer2 = vqa.answer(q2, features);
    ASSERT_EQ(std::stoi(answer2), 2);
    
    // Test "where" question
    dnn::perception::VisualQuestionAnswering::Question q3("Where is the person?");
    auto answer3 = vqa.answer(q3, features);
    ASSERT_FALSE(answer3.empty());
    ASSERT_NE(answer3.find("center"), std::string::npos);
}
