#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace dnn::perception {

// 3D Object Reconstruction from 2D images
class Object3DReconstruction {
public:
    struct Point3D {
        float x, y, z;
        Point3D() : x(0), y(0), z(0) {}
        Point3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    };
    
    struct Point2D {
        float x, y;
        Point2D() : x(0), y(0) {}
        Point2D(float x_, float y_) : x(x_), y(y_) {}
    };
    
    struct Triangle {
        int v1, v2, v3;  // Vertex indices
        Triangle() : v1(0), v2(0), v3(0) {}
        Triangle(int a, int b, int c) : v1(a), v2(b), v3(c) {}
    };
    
    struct Mesh {
        std::vector<Point3D> vertices;
        std::vector<Triangle> faces;
        std::vector<Point3D> normals;
    };
    
    struct Camera {
        Point3D position;
        Point3D look_at;
        float focal_length;
        
        Camera() : focal_length(1.0f) {}
    };
    
    Object3DReconstruction() {}
    
    // Reconstruct 3D from stereo pair
    std::vector<Point3D> stereo_reconstruction(
        const std::vector<Point2D>& left_points,
        const std::vector<Point2D>& right_points,
        const Camera& left_cam,
        const Camera& right_cam) {
        
        std::vector<Point3D> points_3d;
        
        // For each corresponding pair
        for (size_t i = 0; i < std::min(left_points.size(), right_points.size()); i++) {
            Point3D point = triangulate(left_points[i], right_points[i],
                                       left_cam, right_cam);
            points_3d.push_back(point);
        }
        
        return points_3d;
    }
    
    // Depth map to point cloud
    std::vector<Point3D> depth_to_pointcloud(
        const std::vector<std::vector<float>>& depth_map,
        float focal_length = 525.0f) {
        
        std::vector<Point3D> cloud;
        
        int height = depth_map.size();
        if (height == 0) return cloud;
        int width = depth_map[0].size();
        
        float cx = width / 2.0f;
        float cy = height / 2.0f;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float depth = depth_map[y][x];
                
                if (depth > 0.01f) {  // Valid depth
                    Point3D point;
                    point.z = depth;
                    point.x = (x - cx) * depth / focal_length;
                    point.y = (y - cy) * depth / focal_length;
                    cloud.push_back(point);
                }
            }
        }
        
        return cloud;
    }
    
    // Point cloud to mesh (simple Delaunay-like)
    Mesh pointcloud_to_mesh(const std::vector<Point3D>& points) {
        Mesh mesh;
        mesh.vertices = points;
        
        // Simple mesh construction: connect nearby points
        for (size_t i = 0; i < points.size(); i++) {
            for (size_t j = i + 1; j < points.size(); j++) {
                for (size_t k = j + 1; k < points.size(); k++) {
                    // Check if points form a good triangle
                    if (is_valid_triangle(points[i], points[j], points[k])) {
                        mesh.faces.emplace_back(i, j, k);
                    }
                }
            }
        }
        
        // Compute normals
        mesh.normals = compute_normals(mesh);
        
        return mesh;
    }
    
    // Structure from Motion (simplified)
    std::vector<Point3D> structure_from_motion(
        const std::vector<std::vector<Point2D>>& image_points,
        const std::vector<Camera>& cameras) {
        
        if (image_points.empty() || image_points.size() != cameras.size()) {
            return {};
        }
        
        std::vector<Point3D> structure;
        
        // Number of tracked points
        if (image_points[0].empty()) return structure;
        
        size_t num_points = image_points[0].size();
        
        // Triangulate each point from multiple views
        for (size_t pt_idx = 0; pt_idx < num_points; pt_idx++) {
            // Collect observations of this point
            std::vector<Point2D> observations;
            std::vector<Camera> obs_cameras;
            
            for (size_t view = 0; view < image_points.size(); view++) {
                if (pt_idx < image_points[view].size()) {
                    observations.push_back(image_points[view][pt_idx]);
                    obs_cameras.push_back(cameras[view]);
                }
            }
            
            // Multi-view triangulation
            if (observations.size() >= 2) {
                Point3D point = multi_view_triangulate(observations, obs_cameras);
                structure.push_back(point);
            }
        }
        
        return structure;
    }
    
    // Voxel-based reconstruction
    std::vector<Point3D> voxel_carving(
        const std::vector<std::vector<std::vector<float>>>& silhouettes,
        const std::vector<Camera>& cameras,
        float voxel_size = 0.01f) {
        
        std::vector<Point3D> voxels;
        
        // Define voxel grid
        float min_coord = -1.0f;
        float max_coord = 1.0f;
        
        for (float x = min_coord; x <= max_coord; x += voxel_size) {
            for (float y = min_coord; y <= max_coord; y += voxel_size) {
                for (float z = min_coord; z <= max_coord; z += voxel_size) {
                    Point3D voxel(x, y, z);
                    
                    // Check if voxel is consistent with all silhouettes
                    bool consistent = true;
                    
                    for (size_t cam_idx = 0; cam_idx < cameras.size() && 
                         cam_idx < silhouettes.size(); cam_idx++) {
                        Point2D projection = project(voxel, cameras[cam_idx]);
                        
                        // Check if projection is inside silhouette
                        if (!is_in_silhouette(projection, silhouettes[cam_idx])) {
                            consistent = false;
                            break;
                        }
                    }
                    
                    if (consistent) {
                        voxels.push_back(voxel);
                    }
                }
            }
        }
        
        return voxels;
    }
    
private:
    Point3D triangulate(const Point2D& p1, const Point2D& p2,
                       const Camera& cam1, const Camera& cam2) {
        // Simplified triangulation
        float baseline = distance(cam1.position, cam2.position);
        float disparity = std::abs(p1.x - p2.x);
        
        if (disparity < 0.001f) disparity = 0.001f;
        
        Point3D point;
        point.z = (cam1.focal_length * baseline) / disparity;
        point.x = (p1.x * point.z) / cam1.focal_length;
        point.y = (p1.y * point.z) / cam1.focal_length;
        
        return point;
    }
    
    Point3D multi_view_triangulate(const std::vector<Point2D>& observations,
                                   const std::vector<Camera>& cameras) {
        // Simple average of pairwise triangulations
        Point3D sum(0, 0, 0);
        int count = 0;
        
        for (size_t i = 0; i < observations.size(); i++) {
            for (size_t j = i + 1; j < observations.size(); j++) {
                Point3D point = triangulate(observations[i], observations[j],
                                          cameras[i], cameras[j]);
                sum.x += point.x;
                sum.y += point.y;
                sum.z += point.z;
                count++;
            }
        }
        
        if (count > 0) {
            sum.x /= count;
            sum.y /= count;
            sum.z /= count;
        }
        
        return sum;
    }
    
    Point2D project(const Point3D& point, const Camera& camera) {
        // Simple perspective projection
        Point3D relative;
        relative.x = point.x - camera.position.x;
        relative.y = point.y - camera.position.y;
        relative.z = point.z - camera.position.z;
        
        Point2D projection;
        if (std::abs(relative.z) > 0.001f) {
            projection.x = (relative.x * camera.focal_length) / relative.z;
            projection.y = (relative.y * camera.focal_length) / relative.z;
        }
        
        return projection;
    }
    
    bool is_in_silhouette(const Point2D& point,
                         const std::vector<std::vector<float>>& silhouette) {
        int h = silhouette.size();
        if (h == 0) return false;
        int w = silhouette[0].size();
        
        int x = static_cast<int>(point.x + w / 2);
        int y = static_cast<int>(point.y + h / 2);
        
        if (x >= 0 && x < w && y >= 0 && y < h) {
            return silhouette[y][x] > 0.5f;
        }
        
        return false;
    }
    
    bool is_valid_triangle(const Point3D& p1, const Point3D& p2, const Point3D& p3) {
        float d12 = distance(p1, p2);
        float d23 = distance(p2, p3);
        float d31 = distance(p3, p1);
        
        // Triangle shouldn't be too large or degenerate
        float max_edge = 0.1f;
        if (d12 > max_edge || d23 > max_edge || d31 > max_edge) {
            return false;
        }
        
        // Check area (non-degenerate)
        float s = (d12 + d23 + d31) / 2.0f;
        float area = std::sqrt(s * (s - d12) * (s - d23) * (s - d31));
        
        return area > 0.001f;
    }
    
    float distance(const Point3D& p1, const Point3D& p2) {
        float dx = p1.x - p2.x;
        float dy = p1.y - p2.y;
        float dz = p1.z - p2.z;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
    
    std::vector<Point3D> compute_normals(const Mesh& mesh) {
        std::vector<Point3D> normals(mesh.vertices.size());
        
        // For each face, compute normal and add to vertices
        for (const auto& face : mesh.faces) {
            const auto& v1 = mesh.vertices[face.v1];
            const auto& v2 = mesh.vertices[face.v2];
            const auto& v3 = mesh.vertices[face.v3];
            
            // Cross product for normal
            Point3D edge1(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
            Point3D edge2(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z);
            
            Point3D normal;
            normal.x = edge1.y * edge2.z - edge1.z * edge2.y;
            normal.y = edge1.z * edge2.x - edge1.x * edge2.z;
            normal.z = edge1.x * edge2.y - edge1.y * edge2.x;
            
            // Normalize
            float length = std::sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
            if (length > 0) {
                normal.x /= length;
                normal.y /= length;
                normal.z /= length;
            }
            
            // Add to each vertex
            normals[face.v1].x += normal.x;
            normals[face.v1].y += normal.y;
            normals[face.v1].z += normal.z;
            
            normals[face.v2].x += normal.x;
            normals[face.v2].y += normal.y;
            normals[face.v2].z += normal.z;
            
            normals[face.v3].x += normal.x;
            normals[face.v3].y += normal.y;
            normals[face.v3].z += normal.z;
        }
        
        // Normalize all vertex normals
        for (auto& normal : normals) {
            float length = std::sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
            if (length > 0) {
                normal.x /= length;
                normal.y /= length;
                normal.z /= length;
            }
        }
        
        return normals;
    }
};

} // namespace dnn::perception
