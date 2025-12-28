#pragma once
#include "sensory_unit.hpp"
#include <random>

namespace dnn {

    class AudioUnit : public SensoryUnit {
    public:
        AudioUnit() {
            active_features_.resize(384, 0.0);
        }

        std::string name() const override { return "Auditory Cortex (Audio)"; }
        SensoryType type() const override { return SensoryType::Audio; }

        std::vector<double> process_raw(const std::vector<unsigned char>& raw_data) override {
            std::lock_guard<std::mutex> lock(mu_);
            
            // Stub: Convert raw audio segments to frequency-like features
            // In a real implementation, we'd do a Fast Fourier Transform (FFT)
            // For now, we simulate activity based on raw data size/variability
            if (raw_data.empty()) {
                std::fill(active_features_.begin(), active_features_.end(), 0.0);
                return active_features_;
            }

            std::mt19937 gen(raw_data.size());
            std::uniform_real_distribution<> dis(-1.0, 1.0);

            for (size_t i = 0; i < active_features_.size(); ++i) {
                // Mix spectral noise with a hint of the actual data
                double data_hint = (raw_data[i % raw_data.size()] / 255.0) * 2.0 - 1.0;
                active_features_[i] = 0.7 * dis(gen) + 0.3 * data_hint;
            }

            return active_features_;
        }
    };

} // namespace dnn
