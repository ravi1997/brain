#include <gtest/gtest.h>
#include "perception/emotion_speech.hpp"
#include "perception/sensor_fusion.hpp"
#include "reasoning/decision_theory.hpp"
#include "reasoning/cbr.hpp"
#include "nlu/open_ie.hpp"

// Emotion Recognition from Speech Test
TEST(Phase1_Perception, EmotionRecognitionSpeech) {
    dnn::perception::EmotionRecognitionSpeech ers;
    
    // Simulate audio signal
    std::vector<float> audio(1000);
    for (size_t i = 0; i < audio.size(); i++) {
        audio[i] = 0.5f * std::sin(2.0f * 3.14159f * i / 100.0f);  // 160 Hz sine wave
    }
    
    auto features = ers.extract_features(audio);
    ASSERT_GT(features.pitch_mean, 0);
    
    auto emotion = ers.recognize(features);
    std::string emotion_str = ers.emotion_to_string(emotion);
    ASSERT_FALSE(emotion_str.empty());
}

// Multi-Sensor Fusion (Kalman Filter) Test
TEST(Phase1_Perception, MultiSensorFusion) {
    dnn::perception::MultiSensorFusion fusion(3);  // 3D state
    
    fusion.add_sensor(0, 1.0f);  // High reliability
    fusion.add_sensor(1, 0.5f);  // Lower reliability
    
    fusion.update_sensor(0, {1.0f, 2.0f, 3.0f});
    fusion.update_sensor(1, {1.1f, 2.1f, 3.1f});
    
    auto fused = fusion.fuse();
    ASSERT_EQ(fused.size(), 3);
}

// Decision Theory Test
TEST(Phase1_Reasoning, DecisionTheory) {
    dnn::reasoning::DecisionTheory dt;
    
    std::vector<dnn::reasoning::DecisionTheory::Decision> decisions;
    
    // Decision 1: Safe option
    dnn::reasoning::DecisionTheory::Decision d1("Safe");
    d1.outcomes = {{"Win small", 10, 0.9f}, {"Lose small", -2, 0.1f}};
    decisions.push_back(d1);
    
    // Decision 2: Risky option
    dnn::reasoning::DecisionTheory::Decision d2("Risky");
    d2.outcomes = {{"Win big", 50, 0.3f}, {"Lose big", -20, 0.7f}};
    decisions.push_back(d2);
    
    auto best = dt.choose_best(decisions);
    ASSERT_FALSE(best.action.empty());
    ASSERT_TRUE(best.expected_utility != 0);
}

// Case-Based Reasoning Test
TEST(Phase1_Reasoning, CaseBasedReasoning) {
    dnn::reasoning::CaseBasedReasoning cbr;
    
    // Add past cases
    cbr.add_case({"Problem A", {1.0f, 2.0f, 3.0f}, "Solution X", 0.9f});
    cbr.add_case({"Problem B", {1.1f, 2.1f, 3.1f}, "Solution X", 0.8f});
    cbr.add_case({"Problem C", {5.0f, 6.0f, 7.0f}, "Solution Y", 0.95f});
    
    // Query with similar problem
    auto solution = cbr.solve({1.05f, 2.05f, 3.05f}, "New problem");
    ASSERT_FALSE(solution.empty());
    ASSERT_EQ(solution, "Solution X");  // Should retrieve similar cases
}

// Open Information Extraction Test
TEST(Phase1_NLU, OpenIE) {
    dnn::nlu::OpenIE oie;
    
    std::string text = "Barack Obama was born in Hawaii. He works for the government.";
    auto extractions = oie.extract(text);
    
    ASSERT_GT(extractions.size(), 0);
    
    bool found_born = false;
    for (const auto& ext : extractions) {
        if (ext.relation.find("born") != std::string::npos ||
            ext.relation.find("was") != std::string::npos) {
            found_born = true;
            break;
        }
    }
    ASSERT_TRUE(found_born);
}
