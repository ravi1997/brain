#pragma once
#include "sensory_unit.hpp"
#include <vector>
#include <string>

namespace dnn {

    class TactileUnit : public SensoryUnit {
    public:
        TactileUnit() {
            active_features_.resize(16, 0.0); // 16 sensors
        }

        std::string name() const override { return "Somatosensory Cortex (Tactile)"; }
        SensoryType type() const override { return SensoryType::Tactile; }
        
        // Data: [Pressure_1, Temp_1, Pressure_2, Temp_2, ...]
        std::vector<double> process_raw(const std::vector<unsigned char>& raw_data) override {
            std::lock_guard<std::mutex> lock(mu_);
            
            pain_signal_ = false;
            
            for (size_t i = 0; i < active_features_.size() && i < raw_data.size(); ++i) {
                // Normalize 0-255 -> 0-1
                active_features_[i] = raw_data[i] / 255.0;
                
                // Feature 4 Logic: Pain Threshold
                // Even indices are Pressure
                if (i % 2 == 0 && active_features_[i] > 0.95) {
                    pain_signal_ = true;
                }
            }
            return active_features_;
        }

        bool is_in_pain() const {
             std::lock_guard<std::mutex> lock(mu_);
             return pain_signal_;
        }

    private:
        bool pain_signal_ = false;
    };

} // namespace dnn
