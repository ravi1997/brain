#pragma once
#include "sensory_unit.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace dnn {

    struct Coordinates {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double heading = 0.0; // Radians
    };

    class SpatialUnit : public SensoryUnit {
    public:
        SpatialUnit() {
            active_features_.resize(384, 0.0);
        }

        std::string name() const override { return "Spatial Cortex (Lidar)"; }
        SensoryType type() const override { return SensoryType::Lidar; }

        // Process "Lidar" data: vector of distances
        std::vector<double> process_raw(const std::vector<unsigned char>& raw_data) override {
            // Simulate processing 360 degrees of depth data
            // For now, mapping raw bytes to simplified obstacle vectors
            
            std::lock_guard<std::mutex> lock(mu_);
            
            if (raw_data.empty()) {
                // Return last known state or zeros
                return active_features_; 
            }

            // Map simulated lidar points to feature vector
            // E.g., detected obstacles in 8 sectors
            size_t sector_size = raw_data.size() / 8;
            if (sector_size == 0) sector_size = 1;

            for (size_t i = 0; i < 8 && i * sector_size < raw_data.size(); ++i) {
                double avg_dist = 0.0;
                for (size_t j = 0; j < sector_size; ++j) {
                    avg_dist += raw_data[i * sector_size + j];
                }
                avg_dist /= sector_size;
                
                // Map to features (spread across the vector)
                size_t start_idx = i * (active_features_.size() / 8);
                size_t end_idx = start_idx + (active_features_.size() / 8);
                
                // Normalize 0-255 to 0.0-1.0
                double norm_dist = avg_dist / 255.0;
                
                for (size_t k = start_idx; k < end_idx; ++k) {
                    active_features_[k] = norm_dist;
                }
            }

            return active_features_;
        }

        void update_position(double dx, double dy, double dtheta) {
            std::lock_guard<std::mutex> lock(mu_);
            coords_.x += dx;
            coords_.y += dy;
            coords_.heading += dtheta;
        }

        Coordinates get_position() const {
             std::lock_guard<std::mutex> lock(mu_);
             return coords_;
        }

    private:
        Coordinates coords_;
    };

} // namespace dnn
