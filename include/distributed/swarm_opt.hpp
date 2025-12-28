#pragma once
#include <vector>
#include <random>
#include <functional>
#include <limits>
#include <cmath>

namespace dnn::distributed{

// Particle Swarm Optimization for hyperparameter tuning
class SwarmOptimization {
public:
    using ObjectiveFunction = std::function<float(const std::vector<float>&)>;
    
    struct Config {
        int num_particles;
        int max_iterations;
        float inertia;
        float cognitive;
        float social;
        
        Config() : num_particles(30), max_iterations(100), 
                  inertia(0.7f), cognitive(1.5f), social(1.5f) {}
    };
    
    SwarmOptimization(int dimensions, const std::vector<float>& lower_bounds,
                     const std::vector<float>& upper_bounds, const Config& config = Config())
        : dim_(dimensions), lower_(lower_bounds), upper_(upper_bounds), 
          config_(config), gen_(std::random_device{}()) {}
    
    // Optimize objective function (minimization)
    std::vector<float> optimize(ObjectiveFunction objective) {
        initialize_swarm();
        
        global_best_value_ = std::numeric_limits<float>::max();
        
        for (int iter = 0; iter < config_.max_iterations; iter++) {
            for (int i = 0; i < config_.num_particles; i++) {
                // Evaluate fitness
                float fitness = objective(particles_[i].position);
                
                // Update personal best
                if (fitness < particles_[i].best_value) {
                    particles_[i].best_value = fitness;
                    particles_[i].best_position = particles_[i].position;
                }
                
                // Update global best
                if (fitness < global_best_value_) {
                    global_best_value_ = fitness;
                    global_best_position_ = particles_[i].position;
                }
            }
            
            // Update velocities and positions
            update_swarm(iter);
        }
        
        return global_best_position_;
    }
    
    // Get best fitness value found
    float get_best_value() const {
        return global_best_value_;
    }
    
private:
    struct Particle {
        std::vector<float> position;
        std::vector<float> velocity;
        std::vector<float> best_position;
        float best_value;
        
        Particle(int dim) 
            : position(dim), velocity(dim), best_position(dim),
              best_value(std::numeric_limits<float>::max()) {}
    };
    
    int dim_;
    std::vector<float> lower_;
    std::vector<float> upper_;
    Config config_;
    std::mt19937 gen_;
    
    std::vector<Particle> particles_;
    std::vector<float> global_best_position_;
    float global_best_value_;
    
    void initialize_swarm() {
        particles_.clear();
        particles_.reserve(config_.num_particles);
        
        std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
        
        for (int i = 0; i < config_.num_particles; i++) {
            Particle p(dim_);
            
            for (int d = 0; d < dim_; d++) {
                // Random position in bounds
                p.position[d] = lower_[d] + uniform(gen_) * (upper_[d] - lower_[d]);
                
                // Random velocity
                float range = (upper_[d] - lower_[d]) * 0.1f;
                p.velocity[d] = -range + uniform(gen_) * 2 * range;
                
                p.best_position[d] = p.position[d];
            }
            
            particles_.push_back(p);
        }
        
        global_best_position_.resize(dim_);
    }
    
    void update_swarm(int iteration) {
        std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
        
        // Adaptive inertia weight
        float w = config_.inertia - (config_.inertia - 0.4f) * iteration / config_.max_iterations;
        
        for (auto& p : particles_) {
            for (int d = 0; d < dim_; d++) {
                float r1 = uniform(gen_);
                float r2 = uniform(gen_);
                
                // Update velocity
                // v = w*v + c1*r1*(pbest - x) + c2*r2*(gbest - x)
                p.velocity[d] = w * p.velocity[d]
                    + config_.cognitive * r1 * (p.best_position[d] - p.position[d])
                    + config_.social * r2 * (global_best_position_[d] - p.position[d]);
                
                // Clamp velocity
                float max_vel = (upper_[d] - lower_[d]) * 0.2f;
                p.velocity[d] = std::max(-max_vel, std::min(max_vel, p.velocity[d]));
                
                // Update position
                p.position[d] += p.velocity[d];
                
                // Clamp to bounds
                p.position[d] = std::max(lower_[d], std::min(upper_[d], p.position[d]));
            }
        }
    }
};

} // namespace dnn::distributed
