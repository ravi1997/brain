#pragma once
#include <vector>
#include <cmath>
#include <random>
#include <functional>

namespace dnn::optimization {

// Ant Colony Optimization for combinatorial problems
class AntColonyOptimization {
public:
    using CostFunction = std::function<float(const std::vector<int>&)>;
    
    AntColonyOptimization(int num_ants = 50, int num_nodes = 10,
                         float alpha = 1.0f, float beta = 2.0f, 
                         float evaporation = 0.5f)
        : num_ants_(num_ants), num_nodes_(num_nodes),
          alpha_(alpha), beta_(beta), evaporation_(evaporation),
          gen_(std::random_device{}()) {
        
        // Initialize pheromone matrix
        pheromones_.resize(num_nodes * num_nodes, 1.0f);
    }
    
    // Optimize path/solution using ACO
    std::vector<int> optimize(CostFunction cost_fn, int iterations = 100) {
        std::vector<int> best_solution;
        float best_cost = std::numeric_limits<float>::infinity();
        
        for (int iter = 0; iter < iterations; iter++) {
            std::vector<std::vector<int>> ant_solutions;
            std::vector<float> ant_costs;
            
            // Each ant constructs a solution
            for (int ant = 0; ant < num_ants_; ant++) {
                auto solution = construct_solution();
                float cost = cost_fn(solution);
                
                ant_solutions.push_back(solution);
                ant_costs.push_back(cost);
                
                if (cost < best_cost) {
                    best_cost = cost;
                    best_solution = solution;
                }
            }
            
            // Update pheromones
            update_pheromones(ant_solutions, ant_costs);
        }
        
        return best_solution;
    }
    
private:
    int num_ants_;
    int num_nodes_;
    float alpha_;        // Pheromone importance
    float beta_;         // Heuristic importance
    float evaporation_;
    std::vector<float> pheromones_;
    std::mt19937 gen_;
    
    std::vector<int> construct_solution() {
        std::vector<int> solution;
        std::vector<bool> visited(num_nodes_, false);
        
        // Start at random node
        int current = gen_() % num_nodes_;
        solution.push_back(current);
        visited[current] = true;
        
        // Visit all nodes
        for (int step = 1; step < num_nodes_; step++) {
            int next = select_next_node(current, visited);
            solution.push_back(next);
            visited[next] = true;
            current = next;
        }
        
        return solution;
    }
    
    int select_next_node(int current, const std::vector<bool>& visited) {
        std::vector<float> probabilities(num_nodes_, 0.0f);
        float sum = 0.0f;
        
        // Calculate probabilities for unvisited nodes
        for (int next = 0; next < num_nodes_; next++) {
            if (!visited[next]) {
                float pheromone = get_pheromone(current, next);
                float heuristic = 1.0f;  // Simplified heuristic
                
                probabilities[next] = std::pow(pheromone, alpha_) * 
                                     std::pow(heuristic, beta_);
                sum += probabilities[next];
            }
        }
        
        // Roulette wheel selection
        if (sum == 0) {
            // Fallback: choose random unvisited
            for (int i = 0; i < num_nodes_; i++) {
                if (!visited[i]) return i;
            }
            return 0;
        }
        
        std::uniform_real_distribution<float> dist(0.0f, sum);
        float r = dist(gen_);
        float cumulative = 0.0f;
        
        for (int i = 0; i < num_nodes_; i++) {
            if (!visited[i]) {
                cumulative += probabilities[i];
                if (cumulative >= r) {
                    return i;
                }
            }
        }
        
        return 0;
    }
    
    void update_pheromones(const std::vector<std::vector<int>>& solutions,
                          const std::vector<float>& costs) {
        // Evaporation
        for (auto& p : pheromones_) {
            p *= (1.0f - evaporation_);
        }
        
        // Add new pheromones
        for (size_t ant = 0; ant < solutions.size(); ant++) {
            float deposit = 1.0f / (costs[ant] + 1.0f);
            
            for (size_t i = 0; i < solutions[ant].size() - 1; i++) {
                int from = solutions[ant][i];
                int to = solutions[ant][i + 1];
                add_pheromone(from, to, deposit);
            }
        }
    }
    
    float get_pheromone(int from, int to) const {
        int idx = from * num_nodes_ + to;
        return (idx >= 0 && idx < static_cast<int>(pheromones_.size())) 
               ? pheromones_[idx] : 1.0f;
    }
    
    void add_pheromone(int from, int to, float amount) {
        int idx = from * num_nodes_ + to;
        if (idx >= 0 && idx < static_cast<int>(pheromones_.size())) {
            pheromones_[idx] += amount;
        }
    }
};

} // namespace dnn::optimization
