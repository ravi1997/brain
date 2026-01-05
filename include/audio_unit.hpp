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

        void set_spatial_focus(double left, double right) {
            std::lock_guard<std::mutex> lock(mu_);
            left_balance_ = left;
            right_balance_ = right;
        }

        std::vector<double> process_raw(const std::vector<unsigned char>& raw_data) override {
            std::lock_guard<std::mutex> lock(mu_);
            
            if (raw_data.empty()) {
                std::fill(active_features_.begin(), active_features_.end(), 0.0);
                return active_features_;
            }

            std::mt19937 gen(raw_data.size());
            std::uniform_real_distribution<> dis(-1.0, 1.0);

            for (size_t i = 0; i < active_features_.size(); ++i) {
                double data_hint = (raw_data[i % raw_data.size()] / 255.0) * 2.0 - 1.0;
                
                // If odd index, use right balance, else left
                double balance = (i % 2 == 0) ? left_balance_ : right_balance_;
                active_features_[i] = (0.7 * dis(gen) + 0.3 * data_hint) * balance;
            }

            return active_features_;
        }

    private:
        double left_balance_ = 1.0;
        double right_balance_ = 1.0;
    };

} // namespace dnn
