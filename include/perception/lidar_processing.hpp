#pragma once
#include <vector>
#include <cmath>

namespace dnn::perception {

// LiDAR point cloud processing
class LiDARProcessing {
public:
    struct Point3D {
        float x, y, z;
        float intensity;
        
        Point3D() : x(0), y(0), z(0), intensity(0) {}
        Point3D(float px, float py, float pz, float i = 1.0f) 
            : x(px), y(py), z(pz), intensity(i) {}
    };
    
    LiDARProcessing() {}
    
    // Filter point cloud (remove outliers)
    std::vector<Point3D> filter_outliers(const std::vector<Point3D>& points, 
                                         float max_distance = 50.0f) {
        std::vector<Point3D> filtered;
        
        for (const auto& p : points) {
            float dist = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            if (dist < max_distance) {
                filtered.push_back(p);
            }
        }
        
        return filtered;
    }
    
    // Downsample point cloud using voxel grid
    std::vector<Point3D> downsample(const std::vector<Point3D>& points, 
                                   float voxel_size = 0.5f) {
        // Simple grid-based downsampling
        std::vector<Point3D> downsampled;
        
        for (size_t i = 0; i < points.size(); i += 10) {  // Simple decimation
            downsampled.push_back(points[i]);
        }
        
        return downsampled;
    }
    
    // Detect ground plane
    std::vector<Point3D> detect_ground(const std::vector<Point3D>& points, 
                                      float ground_threshold = 0.2f) {
        std::vector<Point3D> ground_points;
        
        for (const auto& p : points) {
            if (std::abs(p.z) < ground_threshold) {
                ground_points.push_back(p);
            }
        }
        
        return ground_points;
    }
    
    // Cluster points (simple distance-based)
    std::vector<std::vector<Point3D>> cluster(const std::vector<Point3D>& points,
                                             float cluster_distance = 1.0f) {
        std::vector<std::vector<Point3D>> clusters;
        std::vector<bool> visited(points.size(), false);
        
        for (size_t i = 0; i < points.size(); i++) {
            if (visited[i]) continue;
            
            std::vector<Point3D> cluster;
            cluster.push_back(points[i]);
            visited[i] = true;
            
            // Find nearby points
            for (size_t j = i + 1; j < points.size(); j++) {
                if (visited[j]) continue;
                
                float dx = points[i].x - points[j].x;
                float dy = points[i].y - points[j].y;
                float dz = points[i].z - points[j].z;
                float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
                
                if (dist < cluster_distance) {
                    cluster.push_back(points[j]);
                    visited[j] = true;
                }
            }
            
            if (cluster.size() > 5) {  // Minimum cluster size
                clusters.push_back(cluster);
            }
        }
        
        return clusters;
    }
};

} // namespace dnn::perception
