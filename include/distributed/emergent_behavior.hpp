#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>
#include <memory>

namespace dnn::distributed {

// Emergent Behavior Simulation for multi-agent systems
class EmergentBehaviorSimulation {
public:
    struct Agent {
        int id;
        std::vector<float> position;  // 2D or 3D
        std::vector<float> velocity;
        std::vector<float> state;     // Internal state
        std::string behavior_type;
        
        Agent() : id(0) {}
        Agent(int i, const std::vector<float>& pos)
            : id(i), position(pos) {
            velocity.resize(pos.size(), 0.0f);
        }
    };
    
    struct Environment {
        std::vector<float> bounds;  // min/max for each dimension
        std::vector<std::vector<float>> obstacles;
        std::unordered_map<std::string, float> parameters;
        
        Environment() {}
    };
    
    EmergentBehaviorSimulation(int num_agents, int dimensions = 2)
        : num_agents_(num_agents), dimensions_(dimensions) {
        initialize_agents();
    }
    
    // Simulate flocking behavior (Boids)
    void simulate_flocking(float dt = 0.1f) {
        std::vector<std::vector<float>> new_velocities(agents_.size());
        
        for (size_t i = 0; i < agents_.size(); i++) {
            auto& agent = agents_[i];
            
            // Flocking rules
            auto separation = compute_separation(agent);
            auto alignment = compute_alignment(agent);
            auto cohesion = compute_cohesion(agent);
            
            // Combine
            std::vector<float> new_vel(dimensions_, 0.0f);
            
            for (int d = 0; d < dimensions_; d++) {
                new_vel[d] = agent.velocity[d] +
                            0.5f * separation[d] +
                            0.3f * alignment[d] +
                            0.2f * cohesion[d];
                
                // Limit speed
                float max_speed = 1.0f;
                new_vel[d] = std::max(-max_speed, std::min(max_speed, new_vel[d]));
            }
            
            new_velocities[i] = new_vel;
        }
        
        // Update positions and velocities
        for (size_t i = 0; i < agents_.size(); i++) {
            agents_[i].velocity = new_velocities[i];
            
            for (int d = 0; d < dimensions_; d++) {
                agents_[i].position[d] += agents_[i].velocity[d] * dt;
                
                // Wrap around boundaries
                if (environment_.bounds.size() > static_cast<size_t>(d * 2 + 1)) {
                    float min_bound = environment_.bounds[d * 2];
                    float max_bound = environment_.bounds[d * 2 + 1];
                    
                    if (agents_[i].position[d] < min_bound) {
                        agents_[i].position[d] = max_bound;
                    } else if (agents_[i].position[d] > max_bound) {
                        agents_[i].position[d] = min_bound;
                    }
                }
            }
        }
    }
    
    // Simulate swarm intelligence (ant-like foraging)
    void simulate_foraging(const std::vector<std::vector<float>>& food_sources,
                          float dt = 0.1f) {
        for (auto& agent : agents_) {
            // Find nearest food
            float min_dist = std::numeric_limits<float>::infinity();
            std::vector<float> target;
            
            for (const auto& food : food_sources) {
                float dist = distance(agent.position, food);
                if (dist < min_dist) {
                    min_dist = dist;
                    target = food;
                }
            }
            
            if (!target.empty()) {
                // Move towards food
                for (int d = 0; d < dimensions_; d++) {
                    float direction = target[d] - agent.position[d];
                    agent.velocity[d] = 0.5f * direction;
                    agent.position[d] += agent.velocity[d] * dt;
                }
            }
        }
    }
    
    // Simulate opinion dynamics (consensus formation)
    void simulate_opinion_dynamics(float dt = 0.1f) {
        std::vector<std::vector<float>> new_states(agents_.size());
        
        for (size_t i = 0; i < agents_.size(); i++) {
            auto& agent = agents_[i];
            
            // Average opinions of neighbors
            auto neighbors = get_neighbors(agent, 2.0f);
            
            if (agent.state.empty()) {
                agent.state.resize(1, static_cast<float>(rand()) / RAND_MAX);
            }
            
            std::vector<float> new_state = agent.state;
            
            if (!neighbors.empty()) {
                new_state[0] = 0.0f;
                
                for (const auto& neighbor : neighbors) {
                    if (!agents_[neighbor].state.empty()) {
                        new_state[0] += agents_[neighbor].state[0];
                    }
                }
                
                new_state[0] /= neighbors.size();
                
                // Blend with current opinion
                float blend = 0.3f;  // How much to adopt neighbors' opinions
                new_state[0] = (1 - blend) * agent.state[0] + blend * new_state[0];
            }
            
            new_states[i] = new_state;
        }
        
        // Update states
        for (size_t i = 0; i < agents_.size(); i++) {
            agents_[i].state = new_states[i];
        }
    }
    
    // Measure emergent properties
    struct EmergentMetrics {
        float global_alignment;    // How aligned are velocities
        float spatial_clustering;  // How clustered are agents
        float opinion_consensus;   // Opinion agreement level
        float entropy;            // System entropy
        
        EmergentMetrics()
            : global_alignment(0), spatial_clustering(0),
              opinion_consensus(0), entropy(0) {}
    };
    
    EmergentMetrics measure_emergence() {
        EmergentMetrics metrics;
        
        // Global alignment
        std::vector<float> avg_velocity(dimensions_, 0.0f);
        
        for (const auto& agent : agents_) {
            for (int d = 0; d < dimensions_; d++) {
                avg_velocity[d] += agent.velocity[d];
            }
        }
        
        for (auto& v : avg_velocity) {
            v /= agents_.size();
        }
        
        float alignment_sum = 0.0f;
        for (const auto& agent : agents_) {
            float dot = 0.0f;
            float norm1 = 0.0f, norm2 = 0.0f;
            
            for (int d = 0; d < dimensions_; d++) {
                dot += agent.velocity[d] * avg_velocity[d];
                norm1 += agent.velocity[d] * agent.velocity[d];
                norm2 += avg_velocity[d] * avg_velocity[d];
            }
            
            if (norm1 > 0 && norm2 > 0) {
                alignment_sum += dot / (std::sqrt(norm1) * std::sqrt(norm2));
            }
        }
        
        metrics.global_alignment = alignment_sum / agents_.size();
        
        // Opinion consensus
        if (!agents_.empty() && !agents_[0].state.empty()) {
            float mean_opinion = 0.0f;
            for (const auto& agent : agents_) {
                if (!agent.state.empty()) {
                    mean_opinion += agent.state[0];
                }
            }
            mean_opinion /= agents_.size();
            
            float variance = 0.0f;
            for (const auto& agent : agents_) {
                if (!agent.state.empty()) {
                    float diff = agent.state[0] - mean_opinion;
                    variance += diff * diff;
                }
            }
            variance /= agents_.size();
            
            metrics.opinion_consensus = 1.0f / (1.0f + variance);
        }
        
        return metrics;
    }
    
    // Get agents
    const std::vector<Agent>& get_agents() const {
        return agents_;
    }
    
    // Set environment
    void set_environment(const Environment& env) {
        environment_ = env;
    }
    
private:
    int num_agents_;
    int dimensions_;
    std::vector<Agent> agents_;
    Environment environment_;
    
    void initialize_agents() {
        agents_.clear();
        
        for (int i = 0; i < num_agents_; i++) {
            std::vector<float> pos(dimensions_);
            for (auto& p : pos) {
                p = static_cast<float>(rand()) / RAND_MAX * 10.0f;
            }
            
            agents_.emplace_back(i, pos);
            agents_.back().velocity.resize(dimensions_);
            
            for (auto& v : agents_.back().velocity) {
                v = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
            }
        }
    }
    
    std::vector<float> compute_separation(const Agent& agent) {
        std::vector<float> steer(dimensions_, 0.0f);
        int count = 0;
        float separation_radius = 1.0f;
        
        for (const auto& other : agents_) {
            if (other.id == agent.id) continue;
            
            float dist = distance(agent.position, other.position);
            
            if (dist < separation_radius && dist > 0) {
                std::vector<float> diff(dimensions_);
                for (int d = 0; d < dimensions_; d++) {
                    diff[d] = agent.position[d] - other.position[d];
                    diff[d] /= (dist * dist);  // Weight by distance
                }
                
                for (int d = 0; d < dimensions_; d++) {
                    steer[d] += diff[d];
                }
                count++;
            }
        }
        
        if (count > 0) {
            for (auto& s : steer) {
                s /= count;
            }
        }
        
        return steer;
    }
    
    std::vector<float> compute_alignment(const Agent& agent) {
        std::vector<float> avg_vel(dimensions_, 0.0f);
        int count = 0;
        float neighbor_radius = 2.0f;
        
        for (const auto& other : agents_) {
            if (other.id == agent.id) continue;
            
            float dist = distance(agent.position, other.position);
            
            if (dist < neighbor_radius) {
                for (int d = 0; d < dimensions_; d++) {
                    avg_vel[d] += other.velocity[d];
                }
                count++;
            }
        }
        
        if (count > 0) {
            for (auto& v : avg_vel) {
                v /= count;
            }
        }
        
        return avg_vel;
    }
    
    std::vector<float> compute_cohesion(const Agent& agent) {
        std::vector<float> center(dimensions_, 0.0f);
        int count = 0;
        float neighbor_radius = 2.0f;
        
        for (const auto& other : agents_) {
            if (other.id == agent.id) continue;
            
            float dist = distance(agent.position, other.position);
            
            if (dist < neighbor_radius) {
                for (int d = 0; d < dimensions_; d++) {
                    center[d] += other.position[d];
                }
                count++;
            }
        }
        
        if (count > 0) {
            for (auto& c : center) {
                c /= count;
            }
            
            // Steer towards center
            for (int d = 0; d < dimensions_; d++) {
                center[d] = center[d] - agent.position[d];
            }
        }
        
        return center;
    }
    
    std::vector<int> get_neighbors(const Agent& agent, float radius) {
        std::vector<int> neighbors;
        
        for (const auto& other : agents_) {
            if (other.id == agent.id) continue;
            
            if (distance(agent.position, other.position) < radius) {
                neighbors.push_back(other.id);
            }
        }
        
        return neighbors;
    }
    
    float distance(const std::vector<float>& a, const std::vector<float>& b) {
        float sum = 0.0f;
        for (size_t i = 0; i < std::min(a.size(), b.size()); i++) {
            float diff = a[i] - b[i];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }
};

} // namespace dnn::distributed
