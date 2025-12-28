#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>

namespace dnn::perception {

// Simultaneous Localization and Mapping
class SLAM {
public:
    struct Pose {
        float x, y, theta;  // 2D pose
        
        Pose() : x(0), y(0), theta(0) {}
        Pose(float px, float py, float ptheta) : x(px), y(py), theta(ptheta) {}
    };
    
    struct Landmark {
        float x, y;
        int id;
        
        Landmark() : x(0), y(0), id(0) {}
        Landmark(float px, float py, int lid) : x(px), y(py), id(lid) {}
    };
    
    SLAM() : current_pose_(), next_landmark_id_(0) {}
    
    // Update pose with odometry
    void update_odometry(float dx, float dy, float dtheta) {
        current_pose_.x += dx * std::cos(current_pose_.theta) - dy * std::sin(current_pose_.theta);
        current_pose_.y += dx * std::sin(current_pose_.theta) + dy * std::cos(current_pose_.theta);
        current_pose_.theta += dtheta;
        
        // Normalize theta
        while (current_pose_.theta > M_PI) current_pose_.theta -= 2 * M_PI;
        while (current_pose_.theta < -M_PI) current_pose_.theta += 2 * M_PI;
        
        trajectory_.push_back(current_pose_);
    }
    
    // Add observed landmark
    void add_observation(float range, float bearing) {
        // Convert to global coordinates
        float landmark_x = current_pose_.x + range * std::cos(current_pose_.theta + bearing);
        float landmark_y = current_pose_.y + range * std::sin(current_pose_.theta + bearing);
        
        // Simple data association (nearest neighbor)
        int matched_id = find_nearest_landmark(landmark_x, landmark_y, 1.0f);
        
        if (matched_id < 0) {
            // New landmark
            landmarks_.emplace_back(landmark_x, landmark_y, next_landmark_id_++);
        } else {
            // Update existing landmark (simple average)
            auto& landmark = landmarks_[matched_id];
            landmark.x = (landmark.x + landmark_x) / 2.0f;
            landmark.y = (landmark.y + landmark_y) / 2.0f;
        }
    }
    
    // Get current pose
    Pose get_pose() const {
        return current_pose_;
    }
    
    // Get current map
    std::vector<Landmark> get_map() const {
        return landmarks_;
    }
    
    // Get trajectory
    std::vector<Pose> get_trajectory() const {
        return trajectory_;
    }
    
private:
    Pose current_pose_;
    std::vector<Landmark> landmarks_;
    std::vector<Pose> trajectory_;
    int next_landmark_id_;
    
    int find_nearest_landmark(float x, float y, float max_distance) {
        int nearest_idx = -1;
        float min_dist = max_distance;
        
        for (size_t i = 0; i < landmarks_.size(); i++) {
            float dx = landmarks_[i].x - x;
            float dy = landmarks_[i].y - y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist < min_dist) {
                min_dist = dist;
                nearest_idx = i;
            }
        }
        
        return nearest_idx;
    }
};

} // namespace dnn::perception
