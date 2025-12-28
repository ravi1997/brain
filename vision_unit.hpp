#pragma once
#include "sensory_unit.hpp"
#include "dnn.hpp"
#include <memory>

namespace dnn {

    class VisionUnit : public SensoryUnit {
    public:
        VisionUnit(const std::vector<std::size_t>& feature_dims) {
            // Internal network to compress visual raw data into thought space
            compression_net_ = std::make_unique<NeuralNetwork>(feature_dims);
            active_features_.resize(feature_dims.back(), 0.0);
        }

        std::string name() const override { return "Ocular Interface (Vision)"; }
        SensoryType type() const override { return SensoryType::Vision; }

        std::vector<double> process_raw(const std::vector<unsigned char>& raw_data) override {
            if (!active_) return {};

            // 1. Convert raw bytes to normalized doubles (simple grayscale/flattened for now)
            std::vector<double> input(raw_data.begin(), raw_data.end());
            for (auto& val : input) val /= 255.0;

            // 2. Pass through compression network
            std::vector<double> features = compression_net_->predict(input);

            // 3. Update state
            std::lock_guard<std::mutex> lock(mu_);
            active_features_ = features;
            return active_features_;
        }

    private:
        std::unique_ptr<NeuralNetwork> compression_net_;
    };

} // namespace dnn
