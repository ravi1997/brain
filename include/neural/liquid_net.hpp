#pragma once
#include <vector>
#include <cmath>
#include <functional>

namespace dnn::neural {

// Liquid Neural Networks (Continuous-time RNN with ODE dynamics)
class LiquidNeuralNetwork {
public:
    struct LiquidNeuron {
        float state;          // Current neuron state
        float time_constant;  // Time constant tau
        float bias;
        
        LiquidNeuron() : state(0.0f), time_constant(1.0f), bias(0.0f) {}
    };
    
    LiquidNeuralNetwork(int num_neurons = 64, float dt = 0.1f)
        : num_neurons_(num_neurons), dt_(dt) {
        neurons_.resize(num_neurons);
        weights_.resize(num_neurons * num_neurons, 0.0f);
        
        // Initialize with small random weights
        for (auto& w : weights_) {
            w = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f;
        }
        
        // Initialize time constants
        for (auto& neuron : neurons_) {
            neuron.time_constant = 0.5f + static_cast<float>(rand()) / RAND_MAX;
            neuron.bias = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
        }
    }
    
    // Forward pass with continuous-time dynamics
    std::vector<float> forward(const std::vector<float>& input, int num_steps = 10) {
        // Set input as initial states
        for (size_t i = 0; i < std::min(input.size(), neurons_.size()); i++) {
            neurons_[i].state = input[i];
        }
        
        // Integrate ODE for num_steps
        for (int step = 0; step < num_steps; step++) {
            update_dynamics();
        }
        
        // Return neuron states
        std::vector<float> output(num_neurons_);
        for (int i = 0; i < num_neurons_; i++) {
            output[i] = neurons_[i].state;
        }
        
        return output;
    }
    
    // Update dynamics using Euler integration
    void update_dynamics() {
        std::vector<float> derivatives(num_neurons_);
        
        // Compute derivatives for all neurons
        for (int i = 0; i < num_neurons_; i++) {
            float weighted_input = neurons_[i].bias;
            
            // Sum weighted inputs from other neurons
            for (int j = 0; j < num_neurons_; j++) {
                weighted_input += weights_[i * num_neurons_ + j] * 
                                 activation(neurons_[j].state);
            }
            
            // Liquid neuron ODE: ds/dt = (-s + f(input)) / tau
            derivatives[i] = (-neurons_[i].state + activation(weighted_input)) / 
                            neurons_[i].time_constant;
        }
        
        // Update states using Euler integration
        for (int i = 0; i < num_neurons_; i++) {
            neurons_[i].state += dt_ * derivatives[i];
        }
    }
    
    // Activation function (tanh)
    float activation(float x) const {
        return std::tanh(x);
    }
    
    // Reset neuron states
    void reset() {
        for (auto& neuron : neurons_) {
            neuron.state = 0.0f;
        }
    }
    
    // Set time constants (for adaptivity)
    void set_time_constants(const std::vector<float>& time_constants) {
        for (size_t i = 0; i < std::min(time_constants.size(), neurons_.size()); i++) {
            neurons_[i].time_constant = time_constants[i];
        }
    }
    
    // Get neuron states
    std::vector<float> get_states() const {
        std::vector<float> states(num_neurons_);
        for (int i = 0; i < num_neurons_; i++) {
            states[i] = neurons_[i].state;
        }
        return states;
    }
    
    // Compute energy (Lyapunov function)
    float compute_energy() const {
        float energy = 0.0f;
        
        for (int i = 0; i < num_neurons_; i++) {
            for (int j = 0; j < num_neurons_; j++) {
                energy -= 0.5f * weights_[i * num_neurons_ + j] * 
                         neurons_[i].state * neurons_[j].state;
            }
            energy -= neurons_[i].bias * neurons_[i].state;
        }
        
        return energy;
    }
    
private:
    int num_neurons_;
    float dt_;  // Time step for integration
    std::vector<LiquidNeuron> neurons_;
    std::vector<float> weights_;
};

} // namespace dnn::neural
