#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <atomic>

namespace dnn {

    enum class SensoryType {
        Vision,
        Audio,
        Lidar,
        Tactile,
        Internal // Proprioception
    };

    class SensoryUnit {
    public:
        virtual ~SensoryUnit() = default;

        virtual std::string name() const = 0;
        virtual SensoryType type() const = 0;

        // Process raw input and convert to a feature vector (e.g. 384-dim)
        virtual std::vector<double> process_raw(const std::vector<unsigned char>& raw_data) = 0;
        
        // Return current activity buffer for visualization
        virtual std::vector<double> get_current_activity() const {
            std::lock_guard<std::mutex> lock(mu_);
            return active_features_;
        }

        void set_focus(double level) { focus_level_ = level; }
        double get_focus() const { return focus_level_; }

        bool is_active() const { return active_; }
        void set_active(bool a) { active_ = a; }

    protected:
        mutable std::mutex mu_;
        std::vector<double> active_features_;
        std::atomic<double> focus_level_{1.0}; // 0.0 to 1.0
        std::atomic<bool> active_{true};
    };

} // namespace dnn
