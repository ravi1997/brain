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
            avg_temp_ = 0.0;
            roughness_ = 0.0;
            
            for (size_t i = 0; i < active_features_.size() && i < raw_data.size(); ++i) {
                // Normalize 0-255 -> 0-1
                active_features_[i] = raw_data[i] / 255.0;
                
                // Pressure logic
                if (i % 2 == 0) {
                    if (active_features_[i] > 0.90) pain_signal_ = true; // High pressure = pain
                } else {
                    // Temperature logic
                    avg_temp_ += active_features_[i];
                    if (active_features_[i] > 0.85 || active_features_[i] < 0.15) {
                        pain_signal_ = true; // Extreme heat or cold = pain
                    }
                }

                // Simple Texture Estimation (variance between adjacent sensors)
                if (i > 0) {
                    roughness_ += std::abs(active_features_[i] - active_features_[i-1]);
                }
            }

            if (raw_data.size() > 1) {
                avg_temp_ /= (raw_data.size() / 2.0);
                roughness_ /= raw_data.size();
            }

            return active_features_;
        }

        bool is_in_pain() const {
             std::lock_guard<std::mutex> lock(mu_);
             return pain_signal_;
        }

        double get_temperature() const { return avg_temp_; }
        double get_roughness() const { return roughness_; }

    private:
        bool pain_signal_ = false;
        double avg_temp_ = 0.0;
        double roughness_ = 0.0;
    };

} // namespace dnn
