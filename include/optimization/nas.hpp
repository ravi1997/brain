#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <cmath>

namespace dnn::optimization {

// Neural Architecture Search using evolutionary strategies
class NeuralArchitectureSearch {
public:
    struct Architecture {
        std::vector<int> layer_sizes;      // Neurons per layer
        std::vector<std::string> activations; // Activation functions
        float learning_rate;
        int num_layers;
        float fitness;  // Performance metric
        
        Architecture() : learning_rate(0.001f), num_layers(0), fitness(0.0f) {}
    };
    
    using FitnessFunction = std::function<float(const Architecture&)>;
    
    NeuralArchitectureSearch(int population_size = 20, int max_layers = 10)
        : population_size_(population_size), max_layers_(max_layers),
          gen_(std::random_device{}()) {
        
        activation_options_ = {"relu", "tanh", "sigmoid", "linear"};
    }
    
    // Run evolutionary search
    Architecture search(FitnessFunction fitness_fn, int generations = 50) {
        // Initialize population
        std::vector<Architecture> population = initialize_population();
        
        Architecture best_arch;
        best_arch.fitness = -1e9f;
        
        for (int gen = 0; gen < generations; gen++) {
            // Evaluate fitness
            for (auto& arch : population) {
                arch.fitness = fitness_fn(arch);
                
                if (arch.fitness > best_arch.fitness) {
                    best_arch = arch;
                }
            }
            
            // Selection, crossover, mutation
            population = evolve_population(population);
        }
        
        return best_arch;
    }
    
private:
    int population_size_;
    int max_layers_;
    std::mt19937 gen_;
    std::vector<std::string> activation_options_;
    
    std::vector<Architecture> initialize_population() {
        std::vector<Architecture> population;
        std::uniform_int_distribution<> layer_dist(2, max_layers_);
        std::uniform_int_distribution<> size_dist(16, 512);
        std::uniform_real_distribution<float> lr_dist(0.0001f, 0.01f);
        
        for (int i = 0; i < population_size_; i++) {
            Architecture arch;
            arch.num_layers = layer_dist(gen_);
            arch.learning_rate = lr_dist(gen_);
            
            for (int j = 0; j < arch.num_layers; j++) {
                arch.layer_sizes.push_back(size_dist(gen_));
                arch.activations.push_back(
                    activation_options_[gen_() % activation_options_.size()]
                );
            }
            
            population.push_back(arch);
        }
        
        return population;
    }
    
    std::vector<Architecture> evolve_population(const std::vector<Architecture>& population) {
        // Sort by fitness
        auto sorted_pop = population;
        std::sort(sorted_pop.begin(), sorted_pop.end(),
                 [](const Architecture& a, const Architecture& b) {
                     return a.fitness > b.fitness;
                 });
        
        std::vector<Architecture> new_population;
        
        // Elitism: keep top 20%
        int elite_count = population_size_ / 5;
        for (int i = 0; i < elite_count; i++) {
            new_population.push_back(sorted_pop[i]);
        }
        
        // Crossover and mutation
        std::uniform_int_distribution<> parent_dist(0, elite_count - 1);
        
        while (new_population.size() < population_size_) {
            int parent1_idx = parent_dist(gen_);
            int parent2_idx = parent_dist(gen_);
            
            Architecture child = crossover(sorted_pop[parent1_idx], sorted_pop[parent2_idx]);
            child = mutate(child);
            
            new_population.push_back(child);
        }
        
        return new_population;
    }
    
    Architecture crossover(const Architecture& parent1, const Architecture& parent2) {
        Architecture child;
        
        // Inherit from parent1 or parent2
        if (gen_() % 2 == 0) {
            child.num_layers = parent1.num_layers;
            child.layer_sizes = parent1.layer_sizes;
            child.activations = parent1.activations;
        } else {
            child.num_layers = parent2.num_layers;
            child.layer_sizes = parent2.layer_sizes;
            child.activations = parent2.activations;
        }
        
        // Mix learning rates
        child.learning_rate = (parent1.learning_rate + parent2.learning_rate) / 2.0f;
        
        return child;
    }
    
    Architecture mutate(Architecture arch) {
        std::uniform_real_distribution<float> mutation_prob(0.0f, 1.0f);
        
        // Mutate layer sizes
        for (auto& size : arch.layer_sizes) {
            if (mutation_prob(gen_) < 0.1f) {
                std::uniform_int_distribution<> size_dist(16, 512);
                size = size_dist(gen_);
            }
        }
        
        // Mutate activations
        for (auto& act : arch.activations) {
            if (mutation_prob(gen_) < 0.1f) {
                act = activation_options_[gen_() % activation_options_.size()];
            }
        }
        
        // Mutate learning rate
        if (mutation_prob(gen_) < 0.1f) {
            std::uniform_real_distribution<float> lr_dist(0.0001f, 0.01f);
            arch.learning_rate = lr_dist(gen_);
        }
        
        return arch;
    }
};

} // namespace dnn::optimization
