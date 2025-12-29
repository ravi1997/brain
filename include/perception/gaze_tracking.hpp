#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace dnn::perception {

// Gaze Tracking using eye landmarks and regression
class GazeTracking {
public:
    struct EyeLandmarks {
        std::vector<float> left_eye;   // 6 points (x,y pairs)
        std::vector<float> right_eye;  // 6 points (x,y pairs)
        std::vector<float> pupil_left;  // (x, y)
        std::vector<float> pupil_right; // (x, y)
        
        EyeLandmarks() {
            left_eye.resize(12, 0.0f);
            right_eye.resize(12, 0.0f);
            pupil_left.resize(2, 0.0f);
            pupil_right.resize(2, 0.0f);
        }
    };
    
    struct GazeVector {
        float yaw;    // Horizontal angle
        float pitch;  // Vertical angle
        float x, y;   // Screen coordinates (normalized 0-1)
        float confidence;
        
        GazeVector() : yaw(0), pitch(0), x(0.5f), y(0.5f), confidence(0) {}
    };
    
    struct HeadPose {
        float roll, pitch, yaw;
        
        HeadPose() : roll(0), pitch(0), yaw(0) {}
    };
    
    GazeTracking() {
        // Initialize calibration parameters
        calibration_offset_x_ = 0.0f;
        calibration_offset_y_ = 0.0f;
        calibration_scale_ = 1.0f;
    }
    
    // Estimate gaze from eye landmarks
    GazeVector estimate_gaze(const EyeLandmarks& landmarks, 
                            const HeadPose& head_pose = HeadPose()) {
        GazeVector gaze;
        
        // Compute eye aspect ratios
        float left_ear = compute_eye_aspect_ratio(landmarks.left_eye);
        float right_ear = compute_eye_aspect_ratio(landmarks.right_eye);
        
        // Check if eyes are open
        if (left_ear < 0.2f || right_ear < 0.2f) {
            gaze.confidence = 0.0f;
            return gaze;  // Eyes closed
        }
        
        // Estimate gaze from pupil position relative to eye corners
        auto left_gaze = estimate_pupil_gaze(landmarks.left_eye, landmarks.pupil_left);
        auto right_gaze = estimate_pupil_gaze(landmarks.right_eye, landmarks.pupil_right);
        
        // Average both eyes
        gaze.yaw = (left_gaze.yaw + right_gaze.yaw) / 2.0f;
        gaze.pitch = (left_gaze.pitch + right_gaze.pitch) / 2.0f;
        
        // Compensate for head pose
        gaze.yaw -= head_pose.yaw;
        gaze.pitch -= head_pose.pitch;
        
        // Convert to screen coordinates
        gaze_to_screen(gaze);
        
        // Apply calibration
        apply_calibration(gaze);
        
        gaze.confidence = (left_ear + right_ear) / 2.0f;
        
        return gaze;
    }
    
    // Calibrate gaze tracking with known points
    void calibrate(const std::vector<EyeLandmarks>& landmark_samples,
                  const std::vector<std::pair<float, float>>& ground_truth_points) {
        if (landmark_samples.size() != ground_truth_points.size() || 
            landmark_samples.empty()) {
            return;
        }
        
        // Collect estimated vs ground truth
        std::vector<float> est_x, est_y, gt_x, gt_y;
        
        for (size_t i = 0; i < landmark_samples.size(); i++) {
            auto gaze = estimate_gaze(landmark_samples[i]);
            est_x.push_back(gaze.x);
            est_y.push_back(gaze.y);
            gt_x.push_back(ground_truth_points[i].first);
            gt_y.push_back(ground_truth_points[i].second);
        }
        
        // Compute linear regression for calibration
        float mean_est_x = 0, mean_est_y = 0, mean_gt_x = 0, mean_gt_y = 0;
        int n = est_x.size();
        
        for (int i = 0; i < n; i++) {
            mean_est_x += est_x[i];
            mean_est_y += est_y[i];
            mean_gt_x += gt_x[i];
            mean_gt_y += gt_y[i];
        }
        
        mean_est_x /= n;
        mean_est_y /= n;
        mean_gt_x /= n;
        mean_gt_y /= n;
        
        // Simple offset calibration
        calibration_offset_x_ = mean_gt_x - mean_est_x;
        calibration_offset_y_ = mean_gt_y - mean_est_y;
        
        // Scale calibration
        float var_est = 0, var_gt = 0;
        for (int i = 0; i < n; i++) {
            float dx_est = est_x[i] - mean_est_x;
            float dy_est = est_y[i] - mean_est_y;
            float dx_gt = gt_x[i] - mean_gt_x;
            float dy_gt = gt_y[i] - mean_gt_y;
            
            var_est += dx_est * dx_est + dy_est * dy_est;
            var_gt += dx_gt * dx_gt + dy_gt * dy_gt;
        }
        
        if (var_est > 0) {
            calibration_scale_ = std::sqrt(var_gt / var_est);
        }
    }
    
    // Smooth gaze over time
    GazeVector smooth_gaze(const std::vector<GazeVector>& gaze_history,
                          float alpha = 0.3f) {
        if (gaze_history.empty()) {
            return GazeVector();
        }
        
        GazeVector smoothed = gaze_history.back();
        
        if (gaze_history.size() > 1) {
            // Exponential moving average
            for (int i = gaze_history.size() - 2; i >= 0 && 
                 i >= static_cast<int>(gaze_history.size()) - 5; i--) {
                smoothed.x = alpha * gaze_history[i].x + (1 - alpha) * smoothed.x;
                smoothed.y = alpha * gaze_history[i].y + (1 - alpha) * smoothed.y;
                smoothed.yaw = alpha * gaze_history[i].yaw + (1 - alpha) * smoothed.yaw;
                smoothed.pitch = alpha * gaze_history[i].pitch + (1 - alpha) * smoothed.pitch;
            }
        }
        
        return smoothed;
    }
    
private:
    float calibration_offset_x_;
    float calibration_offset_y_;
    float calibration_scale_;
    
    float compute_eye_aspect_ratio(const std::vector<float>& eye_points) {
        // EAR = (|p2-p6| + |p3-p5|) / (2 * |p1-p4|)
        if (eye_points.size() < 12) return 0.0f;
        
        float vertical1 = std::sqrt(
            std::pow(eye_points[2] - eye_points[10], 2) +
            std::pow(eye_points[3] - eye_points[11], 2)
        );
        
        float vertical2 = std::sqrt(
            std::pow(eye_points[4] - eye_points[8], 2) +
            std::pow(eye_points[5] - eye_points[9], 2)
        );
        
        float horizontal = std::sqrt(
            std::pow(eye_points[0] - eye_points[6], 2) +
            std::pow(eye_points[1] - eye_points[7], 2)
        );
        
        if (horizontal == 0) return 0.0f;
        
        return (vertical1 + vertical2) / (2.0f * horizontal);
    }
    
    GazeVector estimate_pupil_gaze(const std::vector<float>& eye_points,
                                  const std::vector<float>& pupil) {
        GazeVector gaze;
        
        if (eye_points.size() < 12 || pupil.size() < 2) {
            return gaze;
        }
        
        // Eye center
        float eye_center_x = (eye_points[0] + eye_points[6]) / 2.0f;
        float eye_center_y = (eye_points[2] + eye_points[10]) / 2.0f;
        
        // Pupil offset from center
        float offset_x = pupil[0] - eye_center_x;
        float offset_y = pupil[1] - eye_center_y;
        
        // Eye width/height
        float eye_width = std::abs(eye_points[6] - eye_points[0]);
        float eye_height = std::abs(eye_points[10] - eye_points[2]);
        
        // Normalize
        if (eye_width > 0) {
            gaze.yaw = (offset_x / eye_width) * 30.0f;  // Scale to degrees
        }
        if (eye_height > 0) {
            gaze.pitch = (offset_y / eye_height) * 20.0f;
        }
        
        return gaze;
    }
    
    void gaze_to_screen(GazeVector& gaze) {
        // Map angles to screen coordinates
        // Assume field of view: yaw ±30°, pitch ±20°
        gaze.x = 0.5f + gaze.yaw / 60.0f;
        gaze.y = 0.5f + gaze.pitch / 40.0f;
        
        // Clamp to [0, 1]
        gaze.x = std::max(0.0f, std::min(1.0f, gaze.x));
        gaze.y = std::max(0.0f, std::min(1.0f, gaze.y));
    }
    
    void apply_calibration(GazeVector& gaze) {
        // Apply learned offset and scale
        gaze.x = (gaze.x - 0.5f) * calibration_scale_ + 0.5f + calibration_offset_x_;
        gaze.y = (gaze.y - 0.5f) * calibration_scale_ + 0.5f + calibration_offset_y_;
        
        // Clamp
        gaze.x = std::max(0.0f, std::min(1.0f, gaze.x));
        gaze.y = std::max(0.0f, std::min(1.0f, gaze.y));
    }
};

} // namespace dnn::perception
