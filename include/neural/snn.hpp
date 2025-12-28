#pragma once
#include <vector>
#include <cmath>
#include <random>

namespace dnn::neural {

// Spiking Neural Network (SNN) with Leaky Integrate-and-Fire neurons
class SpikingNeuralNetwork {
public:
    struct LIFNeuron {
        float membrane_potential;
        float threshold;
        float reset_potential;
        float tau;  // Membrane time constant
        bool spiked;
        
        LIFNeuron() : membrane_potential(0.0f), threshold(1.0f),
                     reset_potential(0.0f), tau(10.0f), spiked(false) {}
    };
    
    SpikingNeuralNetwork(int num_neurons, float dt = 1.0f)
        : num_neurons_(num_neurons), dt_(dt), gen_(std::random_device{}()) {
        
        neurons_.resize(num_neurons);
        weights_.resize(num_neurons * num_neurons, 0.0f);
        
        // Initialize with small random weights
        std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
        for (auto& w : weights_) {
            w = dist(gen_) * 0.1f;
        }
    }
    
    // Update neuron dynamics for one timestep
    void update(const std::vector<float>& input_currents) {
        // Reset spike flags
        for (auto& neuron : neurons_) {
            neuron.spiked = false;
        }
        
        // Update each neuron
        for (int i = 0; i < num_neurons_; i++) {
            // Input current
            float I = i < static_cast<int>(input_currents.size()) ? input_currents[i] : 0.0f;
            
            // Add synaptic currents from other neurons
            for (int j = 0; j < num_neurons_; j++) {
                if (neurons_[j].spiked) {
                    I += weights_[i * num_neurons_ + j];
                }
            }
            
            // Leaky integrate-and-fire dynamics
            // dV/dt = (-V + I) / tau
            float dV = (-neurons_[i].membrane_potential + I) / neurons_[i].tau * dt_;
            neurons_[i].membrane_potential += dV;
            
            // Check for spike
            if (neurons_[i].membrane_potential >= neurons_[i].threshold) {
                neurons_[i].spiked = true;
                neurons_[i].membrane_potential = neurons_[i].reset_potential;
            }
        }
    }
    
    // Get spike train
    std::vector<bool> get_spikes() const {
        std::vector<bool> spikes(num_neurons_);
        for (int i = 0; i < num_neurons_; i++) {
            spikes[i] = neurons_[i].spiked;
        }
        return spikes;
    }
    
    // Get membrane potentials
    std::vector<float> get_membrane_potentials() const {
        std::vector<float> potentials(num_neurons_);
        for (int i = 0; i < num_neurons_; i++) {
            potentials[i] = neurons_[i].membrane_potential;
        }
        return potentials;
    }
    
    // Reset all neurons
    void reset() {
        for (auto& neuron : neurons_) {
            neuron.membrane_potential = neuron.reset_potential;
            neuron.spiked = false;
        }
    }
    
    // STDP learning rule (Spike-Timing Dependent Plasticity)
    void apply_stdp(int pre_neuron, int post_neuron, float time_diff, 
                   float learning_rate = 0.01f) {
        // STDP window
        float delta_w = 0.0f;
        
        if (time_diff > 0) {
            // Pre before post -> potentiation
            delta_w = learning_rate * std::exp(-time_diff / 20.0f);
        } else {
            // Post before pre -> depression
            delta_w = -learning_rate * std::exp(time_diff / 20.0f);
        }
        
        weights_[post_neuron * num_neurons_ + pre_neuron] += delta_w;
    }
    
    // Count total spikes
    int count_spikes() const {
        int count = 0;
        for (const auto& neuron : neurons_) {
            if (neuron.spiked) count++;
        }
        return count;
    }
    
private:
    int num_neurons_;
    float dt_;
    std::vector<LIFNeuron> neurons_;
    std::vector<float> weights_;
    std::mt19937 gen_;
};

} // namespace dnn::neural
