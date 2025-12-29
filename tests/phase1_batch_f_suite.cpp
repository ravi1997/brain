#include <gtest/gtest.h>
#include "../include/neural/neural_modules.hpp"
#include "../include/reasoning/probabilistic_logic.hpp"
#include "../include/neural/gradient_meta_learning.hpp"
#include "../include/perception/gaze_tracking.hpp"
#include "../include/perception/visual_grounding.hpp"

// Neural Module Networks Test
TEST(Phase1_Neural, NeuralModuleNetworks) {
    dnn::neural::NeuralModuleNetwork nmn;
    
    // Build simple module network: Find car -> Relate left -> Answer exists
    auto find_car = std::make_shared<dnn::neural::NeuralModuleNetwork::FindModule>("car");
    auto relate_left = std::make_shared<dnn::neural::NeuralModuleNetwork::RelateModule>("left");
    auto answer = std::make_shared<dnn::neural::NeuralModuleNetwork::AnswerModule>("exists");
    
    int find_idx = nmn.add_module(find_car);
    int relate_idx = nmn.add_module(relate_left, {find_idx});
    nmn.add_module(answer, {relate_idx});
    
    auto result = nmn.execute();
    ASSERT_FALSE(result.empty());
    
    std::string desc = nmn.describe();
    ASSERT_FALSE(desc.empty());
}

// Probabilistic Logical Inference Test
TEST(Phase1_Reasoning, ProbabilisticLogic) {
    dnn::reasoning::ProbabilisticLogic pl;
    
    // Simple Bayesian network: Rain -> Sprinkler -> WetGrass
    pl.add_variable({"Rain", {"yes", "no"}});
    pl.add_variable({"Sprinkler", {"yes", "no"}});
    pl.add_variable({"WetGrass", {"yes", "no"}});
    
    // P(Rain)
    pl.add_cpt("Rain", {}, {
        {"yes", 0.2f},
        {"no", 0.8f}
    });
    
    // P(Sprinkler | Rain)
    pl.add_cpt("Sprinkler", {"Rain"}, {
        {"yes|yes", 0.01f},
        {"no|yes", 0.99f},
        {"yes|no", 0.4f},
        {"no|no", 0.6f}
    });
    
    // Query: P(Rain=yes | WetGrass=yes)
    float prob_rain = pl.infer_sampling("Rain", "yes", {{"Sprinkler", "yes"}}, 100);
    ASSERT_GE(prob_rain, 0.0f);
    ASSERT_LE(prob_rain, 1.0f);
}

// Gradient-based Meta-Learning Test
TEST(Phase1_Neural, GradientMetaLearning) {
    dnn::neural::GradientMetaLearning gml(10, 0.001f, 0.01f);
    
    // Create simple task
    dnn::neural::GradientMetaLearning::Task task;
    task.support_x = {{1.0f, 2.0f, 3.0f}, {2.0f, 3.0f, 4.0f}};
    task.support_y = {{5.0f}, {7.0f}};
    task.query_x = {{1.5f, 2.5f, 3.5f}};
    task.query_y = {{6.0f}};
    
    // Adapt to task
    auto adapted = gml.adapt(task, 3);
    ASSERT_EQ(adapted.size(), 10);
    
    // Evaluate
    float loss = gml.evaluate(task, adapted);
    ASSERT_GE(loss, 0.0f);
}

// Gaze Tracking Test
TEST(Phase1_Perception, GazeTracking) {
    dnn::perception::GazeTracking gt;
    
    // Create mock eye landmarks
    dnn::perception::GazeTracking::EyeLandmarks landmarks;
    
    // Left eye points (simplified)
    landmarks.left_eye = {
        0.2f, 0.3f,  // p1
        0.25f, 0.28f, // p2
        0.3f, 0.28f,  // p3
        0.35f, 0.3f,  // p4
        0.3f, 0.32f,  // p5
        0.25f, 0.32f  // p6
    };
    
    // Right eye (mirror)
    landmarks.right_eye = {
        0.6f, 0.3f,
        0.65f, 0.28f,
        0.7f, 0.28f,
        0.75f, 0.3f,
        0.7f, 0.32f,
        0.65f, 0.32f
    };
    
    // Pupils (centered)
    landmarks.pupil_left = {0.275f, 0.3f};
    landmarks.pupil_right = {0.675f, 0.3f};
    
    auto gaze = gt.estimate_gaze(landmarks);
    ASSERT_GE(gaze.x, 0.0f);
    ASSERT_LE(gaze.x, 1.0f);
    ASSERT_GE(gaze.y, 0.0f);
    ASSERT_LE(gaze.y, 1.0f);
}

// Visual Grounding Test
TEST(Phase1_Perception, VisualGrounding) {
    dnn::perception::VisualGrounding vg;
    
    // Create regions
    std::vector<dnn::perception::VisualGrounding::Region> regions;
    
    dnn::perception::VisualGrounding::Region r1(0.1f, 0.2f, 0.2f, 0.3f);
    r1.description = "person standing";
    regions.push_back(r1);
    
    dnn::perception::VisualGrounding::Region r2(0.6f, 0.5f, 0.3f, 0.2f);
    r2.description = "red car";
    regions.push_back(r2);
    
    // Ground phrase
    dnn::perception::VisualGrounding::Phrase phrase("the red car");
    auto grounded = vg.ground(phrase, regions);
    
    ASSERT_GT(grounded.confidence, 0.0f);
    ASSERT_EQ(grounded.description, "red car");
    
    // Spatial relationship grounding
    auto left_of_car = vg.ground_spatial_relation("left", r2, regions);
    ASSERT_GE(left_of_car.confidence, 0.0f);
}
