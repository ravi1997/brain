#pragma once
#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>

namespace dnn::optimization {

// Neuro-Evolution Strategies for neural network optimization
class NeuroEvolution {
public:
    struct Individual {
        std::vector<float> genome;  // Network weights
        float fitness;
        
        Individual() : fitness(0) {}
        Individual(const std::vector<float>& g, float f = 0)
            : genome(g), fitness(f) {}
    };
    
    struct EvolutionConfig {
        int population_size;
        float mutation_rate;
        float mutation_strength;
        float crossover_rate;
        int elite_count;
        
        EvolutionConfig()
            : population_size(50), mutation_rate(0.1f),
              mutation_strength(0.1f), crossover_rate(0.7f),
              elite_count(5) {}
    };
    
    NeuroEvolution(int genome_size, const EvolutionConfig& config = EvolutionConfig())
        : genome_size_(genome_size), config_(config) {
        initialize_population();
    }
    
    // Evolve population for one generation
    void evolve(std::function<float(const std::vector<float>&)> fitness_function) {
        // Evaluate fitness
        for (auto& individual : population_) {
            individual.fitness = fitness_function(individual.genome);
        }
        
        // Sort by fitness (descending)
        std::sort(population_.begin(), population_.end(),
                 [](const Individual& a, const Individual& b) {
                     return a.fitness > b.fitness;
                 });
        
        // Create next generation
        std::vector<Individual> next_generation;
        
        // Elitism: keep best individuals
        for (int i = 0; i < config_.elite_count && i < static_cast<int>(population_.size()); i++) {
            next_generation.push_back(population_[i]);
        }
        
        // Generate offspring
        while (next_generation.size() < static_cast<size_t>(config_.population_size)) {
            // Select parents using tournament selection
            auto parent1 = tournament_select();
            auto parent2 = tournament_select();
            
            // Crossover
            Individual offspring;
            if (static_cast<float>(rand()) / RAND_MAX < config_.crossover_rate) {
                offspring = crossover(parent1, parent2);
            } else {
                offspring = parent1;
            }
            
            // Mutation
            mutate(offspring);
            
            next_generation.push_back(offspring);
        }
        
        population_ = next_generation;
        generation_++;
    }
    
    // Train for multiple generations
    void train(std::function<float(const std::vector<float>&)> fitness_function,
              int num_generations) {
        for (int gen = 0; gen < num_generations; gen++) {
            evolve(fitness_function);
        }
    }
    
    // Get best individual
    Individual get_best() const {
        auto best = std::max_element(population_.begin(), population_.end(),
                                     [](const Individual& a, const Individual& b) {
                                         return a.fitness < b.fitness;
                                     });
        return best != population_.end() ? *best : Individual();
    }
    
    // Get population statistics
    struct Statistics {
        float best_fitness;
        float average_fitness;
        float worst_fitness;
        float std_dev;
    };
    
    Statistics get_statistics() const {
        Statistics stats;
        
        if (population_.empty()) {
            return stats;
        }
        
        stats.best_fitness = -std::numeric_limits<float>::infinity();
        stats.worst_fitness = std::numeric_limits<float>::infinity();
        float sum = 0.0f;
        
        for (const auto& ind : population_) {
            stats.best_fitness = std::max(stats.best_fitness, ind.fitness);
            stats.worst_fitness = std::min(stats.worst_fitness, ind.fitness);
            sum += ind.fitness;
        }
        
        stats.average_fitness = sum / population_.size();
        
        // Standard deviation
        float variance = 0.0f;
        for (const auto& ind : population_) {
            float diff = ind.fitness - stats.average_fitness;
            variance += diff * diff;
        }
        stats.std_dev = std::sqrt(variance / population_.size());
        
        return stats;
    }
    
    // CMA-ES variant: Covariance Matrix Adaptation
    void cma_es_step(std::function<float(const std::vector<float>&)> fitness_function) {
        // Simplified CMA-ES
        
        // Evaluate population
        for (auto& ind : population_) {
            ind.fitness = fitness_function(ind.genome);
        }
        
        // Sort by fitness
        std::sort(population_.begin(), population_.end(),
                 [](const Individual& a, const Individual& b) {
                     return a.fitness > b.fitness;
                 });
        
        // Compute weighted mean of top individuals
        int mu = config_.population_size / 2;
        std::vector<float> mean(genome_size_, 0.0f);
        
        for (int i = 0; i < mu && i < static_cast<int>(population_.size()); i++) {
            for (int j = 0; j < genome_size_; j++) {
                mean[j] += population_[i].genome[j];
            }
        }
        
        for (auto& m : mean) {
            m /= mu;
        }
        
        // Sample new population around mean
        for (auto& ind : population_) {
            for (int i = 0; i < genome_size_; i++) {
                float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2;
                ind.genome[i] = mean[i] + config_.mutation_strength * noise;
            }
        }
        
        generation_++;
    }
    
private:
    int genome_size_;
    EvolutionConfig config_;
    std::vector<Individual> population_;
    int generation_ = 0;
    
    void initialize_population() {
        population_.clear();
        
        for (int i = 0; i < config_.population_size; i++) {
            Individual ind;
            ind.genome.resize(genome_size_);
            
            // Random initialization
            for (auto& gene : ind.genome) {
                gene = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
            }
            
            population_.push_back(ind);
        }
    }
    
    Individual tournament_select(int tournament_size = 3) {
        Individual best;
        best.fitness = -std::numeric_limits<float>::infinity();
        
        for (int i = 0; i < tournament_size; i++) {
            int idx = rand() % population_.size();
            if (population_[idx].fitness > best.fitness) {
                best = population_[idx];
            }
        }
        
        return best;
    }
    
    Individual crossover(const Individual& parent1, const Individual& parent2) {
        Individual offspring;
        offspring.genome.resize(genome_size_);
        
        // Uniform crossover
        for (int i = 0; i < genome_size_; i++) {
            if (static_cast<float>(rand()) / RAND_MAX < 0.5f) {
                offspring.genome[i] = parent1.genome[i];
            } else {
                offspring.genome[i] = parent2.genome[i];
            }
        }
        
        return offspring;
    }
    
    void mutate(Individual& individual) {
        for (auto& gene : individual.genome) {
            if (static_cast<float>(rand()) / RAND_MAX < config_.mutation_rate) {
                float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2;
                gene += config_.mutation_strength * noise;
            }
        }
    }
};

} // namespace dnn::optimization
