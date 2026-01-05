#pragma once
#include "sensory_unit.hpp"
#include <chrono>
#include <cmath>

namespace dnn {

    class ClockUnit : public SensoryUnit {
    public:
        ClockUnit() {
            active_features_.resize(384, 0.0);
            start_time_ = std::chrono::steady_clock::now();
        }

        std::string name() const override { return "Temporal Cortex (Clock)"; }
        SensoryType type() const override { return SensoryType::Internal; }

        std::vector<double> process_raw(const std::vector<unsigned char>& /* raw_data */) override {
            // Clock doesn't process external raw data, it processes time
            update_features();
            return active_features_;
        }

        void update_features() {
            auto now = std::chrono::steady_clock::now();
            // Use floating point seconds for smooth sine waves
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
            double elapsed = elapsed_ms / 1000.0;
            
            std::lock_guard<std::mutex> lock(mu_);
            // Encode time as sine/cosine waves of different frequencies (Temporal Encoding)
            // Similar to positional encoding in transformers
            for (size_t i = 0; i < active_features_.size(); ++i) {
                double freq = 1.0 / std::pow(10000.0, (2.0 * (i / 2) / (double)active_features_.size()));
                if (i % 2 == 0) {
                    active_features_[i] = std::sin(elapsed * freq);
                } else {
                    active_features_[i] = std::cos(elapsed * freq);
                }
            }
        }

        double get_idle_seconds() const {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::seconds>(now - last_interaction_).count();
        }

        void record_interaction() {
            last_interaction_ = std::chrono::steady_clock::now();
        }

    private:
        std::chrono::steady_clock::time_point start_time_;
        std::chrono::steady_clock::time_point last_interaction_ = std::chrono::steady_clock::now();
    };

} // namespace dnn
