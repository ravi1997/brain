#include <gtest/gtest.h>
#include "../brain.hpp"
#include "../audio_unit.hpp"
#include "../clock_unit.hpp"
#include <cmath>
#include <thread>

TEST(TestSensoryFusion, Registration) {
    Brain brain;
    // Brain registers 3 default units in constructor: Vision, Audio, Clock
    ASSERT_EQ(brain.sensory_inputs.size(), 3);
    
    bool has_audio = false;
    bool has_clock = false;
    for (const auto& unit : brain.sensory_inputs) {
        if (unit->type() == dnn::SensoryType::Audio) has_audio = true;
        if (unit->type() == dnn::SensoryType::Internal) has_clock = true;
    }
    ASSERT_TRUE(has_audio);
    ASSERT_TRUE(has_clock);
}

TEST(TestSensoryFusion, ClockDynamics) {
    dnn::ClockUnit clock;
    
    // Initial state
    auto f1 = clock.process_raw({});
    
    // Sleep to let time pass
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto f2 = clock.process_raw({});
    
    // Vectors should be different due to time passing (sine waves shift)
    double diff = 0.0;
    for (size_t i = 0; i < f1.size(); ++i) {
        diff += std::abs(f1[i] - f2[i]);
    }
    
    ASSERT_GT(diff, 0.1); 
}

TEST(TestSensoryFusion, Weighting) {
    Brain brain;
    
    // Inject data to ensure non-zero activity
    std::vector<unsigned char> dummy_audio(1024, 128); 
    for (auto& unit : brain.sensory_inputs) {
        if (unit->type() == dnn::SensoryType::Audio) {
            unit->process_raw(dummy_audio);
        } else if (unit->type() == dnn::SensoryType::Internal) {
             // Clock auto-updates on access usually, but force update
             static_cast<dnn::ClockUnit*>(unit.get())->update_features();
        }
    }

    // Set Vision focus to high, Audio to low
    // We access sensory_inputs because it's public in Brain (struct/class default) or public in implementation
    for (auto& unit : brain.sensory_inputs) {
        if (unit->type() == dnn::SensoryType::Vision) unit->set_focus(1.0);
        else unit->set_focus(0.0);
    }
    
    auto aggregate_vision = brain.get_aggregate_sensory_input();
    
    // Set Audio focus to high, Vision to low
    for (auto& unit : brain.sensory_inputs) {
        if (unit->type() == dnn::SensoryType::Vision) unit->set_focus(0.0);
        else if (unit->type() == dnn::SensoryType::Audio) unit->set_focus(1.0);
        else unit->set_focus(0.0);
    }
    
    auto aggregate_audio = brain.get_aggregate_sensory_input();
    
    // They should be different
    double diff = 0.0;
    for (size_t i = 0; i < aggregate_vision.size(); ++i) {
        diff += std::abs(aggregate_vision[i] - aggregate_audio[i]);
    }
    
    ASSERT_GT(diff, 0.0); 
}
