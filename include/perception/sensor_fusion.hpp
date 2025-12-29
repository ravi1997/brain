#pragma once
#include <vector>
#include <cmath>

namespace dnn::perception {

// Multi-Sensor Fusion using Kalman Filter
class KalmanFilter {
public:
    KalmanFilter(int state_dim, int measurement_dim)
        : state_dim_(state_dim), measurement_dim_(measurement_dim) {
        
        // Initialize state
        state_.resize(state_dim, 0.0f);
        
        // State covariance matrix (diagonal)
        P_.resize(state_dim * state_dim, 0.0f);
        for (int i = 0; i < state_dim; i++) {
            P_[i * state_dim + i] = 1.0f;
        }
        
        // Process noise covariance (diagonal)
        Q_.resize(state_dim * state_dim, 0.0f);
        for (int i = 0; i < state_dim; i++) {
            Q_[i * state_dim + i] = 0.01f;
        }
        
        // Measurement noise covariance (diagonal)
        R_.resize(measurement_dim * measurement_dim, 0.0f);
        for (int i = 0; i < measurement_dim; i++) {
            R_[i * measurement_dim + i] = 0.1f;
        }
    }
    
    // Prediction step
    void predict(const std::vector<float>& control_input = {}) {
        // Simple prediction: x = x (constant position model)
        // P = P + Q
        for (int i = 0; i < state_dim_ * state_dim_; i++) {
            P_[i] += Q_[i];
        }
    }
    
    // Update step with measurement
    void update(const std::vector<float>& measurement) {
        // Kalman gain: K = P * H^T * (H * P * H^T + R)^-1
        // For simple case where measurement = state: H = I
        
        // Innovation: y = z - H*x
        std::vector<float> innovation(measurement_dim_);
        for (int i = 0; i < measurement_dim_; i++) {
            innovation[i] = measurement[i] - state_[i];
        }
        
        // Simplified Kalman gain (assuming H = I)
        std::vector<float> K(state_dim_, 0.0f);
        for (int i = 0; i < state_dim_ && i < measurement_dim_; i++) {
            float S = P_[i * state_dim_ + i] + R_[i * measurement_dim_ + i];
            K[i] = P_[i * state_dim_ + i] / S;
        }
        
        // Update state: x = x + K*y
        for (int i = 0; i < state_dim_ && i < measurement_dim_; i++) {
            state_[i] += K[i] * innovation[i];
        }
        
        // Update covariance: P = (I - K*H) * P
        for (int i = 0; i < state_dim_ && i < measurement_dim_; i++) {
            P_[i * state_dim_ + i] *= (1.0f - K[i]);
        }
    }
    
    // Get current state estimate
    std::vector<float> get_state() const {
        return state_;
    }
    
    // Set process noise
    void set_process_noise(float noise) {
        for (int i = 0; i < state_dim_; i++) {
            Q_[i * state_dim_ + i] = noise;
        }
    }
    
    // Set measurement noise
    void set_measurement_noise(float noise) {
        for (int i = 0; i < measurement_dim_; i++) {
            R_[i * measurement_dim_ + i] = noise;
        }
    }
    
private:
    int state_dim_;
    int measurement_dim_;
    std::vector<float> state_;       // State estimate
    std::vector<float> P_;           // State covariance
    std::vector<float> Q_;           // Process noise covariance
    std::vector<float> R_;           // Measurement noise covariance
};

// Multi-sensor fusion combining multiple Kalman filters
class MultiSensorFusion {
public:
    MultiSensorFusion(int state_dim) : state_dim_(state_dim) {
        fused_state_.resize(state_dim, 0.0f);
    }
    
    // Add sensor with its Kalman filter
    void add_sensor(int sensor_id, float reliability = 1.0f) {
        filters_[sensor_id] = std::make_shared<KalmanFilter>(state_dim_, state_dim_);
        reliability_[sensor_id] = reliability;
    }
    
    // Update specific sensor
    void update_sensor(int sensor_id, const std::vector<float>& measurement) {
        if (filters_.count(sensor_id)) {
            filters_[sensor_id]->predict();
            filters_[sensor_id]->update(measurement);
        }
    }
    
    // Fuse all sensor estimates
    std::vector<float> fuse() {
        fused_state_.assign(state_dim_, 0.0f);
        float total_weight = 0.0f;
        
        // Weighted average based on reliability
        for (const auto& [id, filter] : filters_) {
            float weight = reliability_[id];
            auto state = filter->get_state();
            
            for (int i = 0; i < state_dim_; i++) {
                fused_state_[i] += weight * state[i];
            }
            total_weight += weight;
        }
        
        // Normalize
        if (total_weight > 0) {
            for (auto& s : fused_state_) {
                s /= total_weight;
            }
        }
        
        return fused_state_;
    }
    
private:
    int state_dim_;
    std::unordered_map<int, std::shared_ptr<KalmanFilter>> filters_;
    std::unordered_map<int, float> reliability_;
    std::vector<float> fused_state_;
};

} // namespace dnn::perception
