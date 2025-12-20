#pragma once

#include <vector>
#include <string>
#include <random>
#include <mutex>
#include <iostream>
#include <memory>

namespace dnn {

    enum class Activation {
        Relu,
        Sigmoid,
        Tanh,
        Linear
    };

    struct PlasticLayer {
        std::size_t in_size{};
        std::size_t out_size{};

        std::vector<double> weights;
        std::vector<double> biases;
        std::vector<double> eligibility_traces;
        std::vector<double> homeostatic_targets;
        std::vector<double> plasticity_rates;
        std::vector<bool> synaptic_pruning_mask;

        std::vector<double> z_cache;
        std::vector<double> a_cache;
        std::vector<std::size_t> indices;

        double hebbian_learning_rate{0.01};
        double homeostatic_strength{0.001};
        double decay_rate{0.95};
        double pruning_threshold{1e-4};

        PlasticLayer() = default;
        PlasticLayer(std::size_t in, std::size_t out, std::mt19937_64 &rng);

        void forward(const std::vector<double> &input,
                     std::vector<double> &z_out,
                     std::vector<double> &a_out,
                     Activation act) const;

        void forward_cache(const std::vector<double> &input, Activation act);

        void backward(const std::vector<double> &input,
                      const std::vector<double> &dL_dout,
                      std::vector<double> &dL_dinput,
                      std::vector<double> &grad_w,
                      std::vector<double> &grad_b,
                      Activation act) const;

        void apply_gradients(const std::vector<double> &grad_w,
                             const std::vector<double> &grad_b,
                             double lr,
                             const std::vector<double> &input,
                             const std::vector<double> &output);

        void consolidate_memory(const std::vector<double>& importance_scores);
        void prune_synapses();
        
        void save(std::ostream &os) const;
        void load(std::istream &is);
    };

    class NeuralNetwork {
    private:
        std::vector<PlasticLayer> plastic_layers_;
        // We simplified and removed the non-plastic layers_ for this task
        // as the brain only uses plasticity.
        
        Activation hidden_activation_;
        Activation output_activation_;
        bool use_plasticity_;
        bool debug_enabled_{false};
        
        std::vector<std::vector<double>> forward_buffers_;

    public:
        NeuralNetwork() = default;
        NeuralNetwork(const std::vector<std::size_t> &layer_sizes,
                      Activation hidden_act = Activation::Relu,
                      Activation output_act = Activation::Linear);

        void set_debug(bool enabled) { debug_enabled_ = enabled; }
        void set_plasticity(bool enabled) { use_plasticity_ = enabled; }
        
        void consolidate_memories(const std::vector<double>& importance_scores);
        void prune_synapses();

        std::size_t input_size() const { return plastic_layers_.empty() ? 0 : plastic_layers_.front().in_size; }
        std::size_t output_size() const { return plastic_layers_.empty() ? 0 : plastic_layers_.back().out_size; }

        std::vector<double> predict(const std::vector<double> &input) const;

        void train(const std::vector<std::vector<double>> &X,
                   const std::vector<std::vector<double>> &Y,
                   int epochs,
                   int batch_size,
                   double learning_rate);
                   
        void save(std::ostream &os) const;
        void load(std::istream &is);
    };

} // namespace dnn
