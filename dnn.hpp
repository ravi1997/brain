// deep_nn.hpp
#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <execution>
#include <iostream>
#include <mutex>
#include <numeric>
#include <random>
#include <string>
#include <vector>

namespace dnn
{

    enum class Activation
    {
        Relu,
        Sigmoid,
        Tanh,
        Linear
    };

    namespace detail
    {

        inline double relu(double x) noexcept
        {
            return x > 0.0 ? x : 0.0;
        }

        inline double relu_deriv(double x) noexcept
        {
            return x > 0.0 ? 1.0 : 0.0;
        }

        inline double sigmoid(double x)
        {
            if (x >= 0.0)
            {
                double e = std::exp(-x);
                return 1.0 / (1.0 + e);
            }
            else
            {
                double e = std::exp(x);
                return e / (1.0 + e);
            }
        }

        inline double sigmoid_deriv(double y) noexcept
        {
            // y = sigmoid(x)
            return y * (1.0 - y);
        }

        inline double tanh_act(double x)
        {
            return std::tanh(x);
        }

        inline double tanh_deriv(double y) noexcept
        {
            // y = tanh(x)
            return 1.0 - y * y;
        }

        inline double activate(double x, Activation a)
        {
            switch (a)
            {
            case Activation::Relu:
                return relu(x);
            case Activation::Sigmoid:
                return sigmoid(x);
            case Activation::Tanh:
                return tanh_act(x);
            case Activation::Linear:
                return x;
            default:
                return x;
            }
        }

        inline double activate_deriv(double x, double y, Activation a)
        {
            // x: pre-activation (z), y: post-activation (a)
            switch (a)
            {
            case Activation::Relu:
                return relu_deriv(x);
            case Activation::Sigmoid:
                return sigmoid_deriv(y);
            case Activation::Tanh:
                return tanh_deriv(y);
            case Activation::Linear:
                return 1.0;
            default:
                return 1.0;
            }
        }

    } // namespace detail

    // Simple logger controlled by macro + runtime flag
    class Logger
    {
    public:
        static std::mutex &mutex()
        {
            static std::mutex m;
            return m;
        }
    };

    // Enhanced layer with synaptic plasticity and memory consolidation
    struct PlasticLayer
    {
        std::size_t in_size{};
        std::size_t out_size{};

        // Primary weights - standard connection strengths
        std::vector<double> weights;
        std::vector<double> biases;

        // Plasticity parameters for Hebbian learning
        std::vector<double> eligibility_traces;  // For temporal credit assignment
        std::vector<double> homeostatic_targets; // For homeostatic regulation
        std::vector<double> plasticity_rates;    // Individual learning rates per synapse
        std::vector<bool> synaptic_pruning_mask; // For synaptic pruning

        // Caches for forward/backward
        std::vector<double> z_cache;      // pre-activation
        std::vector<double> a_cache;      // post-activation
        std::vector<std::size_t> indices; // for parallel loops

        // Plasticity parameters
        double hebbian_learning_rate{0.01};
        double homeostatic_strength{0.001};
        double decay_rate{0.95};
        double pruning_threshold{1e-4};

        PlasticLayer() = default;

        PlasticLayer(std::size_t in, std::size_t out, std::mt19937_64 &rng)
            : in_size(in),
              out_size(out),
              weights(in * out),
              biases(out),
              eligibility_traces(in * out, 0.0),
              homeostatic_targets(out, 0.0),
              plasticity_rates(in * out, 0.01),
              synaptic_pruning_mask(in * out, true),
              z_cache(out),
              a_cache(out),
              indices(out)
        {
            // Kaiming / He-like initialization (simple version)
            std::normal_distribution<double> dist(
                0.0, std::sqrt(2.0 / static_cast<double>(in_size)));

            for (auto &w : weights)
            {
                w = dist(rng);
            }
            std::fill(biases.begin(), biases.end(), 0.0);

            // Initialize homeostatic targets
            std::fill(homeostatic_targets.begin(), homeostatic_targets.end(), 0.0);

            // Initialize plasticity rates with small random values
            std::uniform_real_distribution<double> rate_dist(0.001, 0.02);
            for (auto &rate : plasticity_rates)
            {
                rate = rate_dist(rng);
            }

            std::iota(indices.begin(), indices.end(), std::size_t{0});
        }

        // Forward into provided buffers (z_out, a_out)
        void forward(const std::vector<double> &input,
                     std::vector<double> &z_out,
                     std::vector<double> &a_out,
                     Activation act) const
        {
            assert(input.size() == in_size);
            assert(z_out.size() == out_size);
            assert(a_out.size() == out_size);

            const double *wptr = weights.data();
            const double *bptr = biases.data();
            const double *inptr = input.data();
            double *zptr = z_out.data();
            double *aptr = a_out.data();

            std::for_each(std::execution::par_unseq,
                          indices.begin(), indices.end(),
                          [&](std::size_t j)
                          {
                              double sum = bptr[j];
                              const double *row = wptr + j * in_size;
                              for (std::size_t i = 0; i < in_size; ++i)
                              {
                                  if (synaptic_pruning_mask[j * in_size + i])
                                  {
                                      sum += row[i] * inptr[i];
                                  }
                              }
                              zptr[j] = sum;
                              aptr[j] = detail::activate(sum, act);
                          });
        }

        // Forward using internal caches
        void forward_cache(const std::vector<double> &input,
                           Activation act)
        {
            forward(input, z_cache, a_cache, act);
        }

        // Backward:
        // dL_dout  : derivative of loss wrt this layer's output (a_cache)
        // input    : previous layer activation (a_prev)
        // dL_dinput: output, gradient wrt input (same size as in_size)
        // grad_w   : gradient wrt weights (same size as weights)
        // grad_b   : gradient wrt biases (same size as biases)
        void backward(const std::vector<double> &input,
                      const std::vector<double> &dL_dout,
                      std::vector<double> &dL_dinput,
                      std::vector<double> &grad_w,
                      std::vector<double> &grad_b,
                      Activation act) const
        {
            assert(input.size() == in_size);
            assert(dL_dout.size() == out_size);
            assert(dL_dinput.size() == in_size);
            assert(grad_w.size() == weights.size());
            assert(grad_b.size() == biases.size());

            std::fill(dL_dinput.begin(), dL_dinput.end(), 0.0);
            std::fill(grad_w.begin(), grad_w.end(), 0.0);
            std::fill(grad_b.begin(), grad_b.end(), 0.0);

            const double *inptr = input.data();
            const double *dptr = dL_dout.data();
            const double *zptr = z_cache.data();
            const double *aptr = a_cache.data();

            double *gwptr = grad_w.data();
            double *gbptr = grad_b.data();
            double *dinptr = dL_dinput.data();
            const double *wptr = weights.data();

            // First, compute local delta for each neuron j
            std::vector<double> delta(out_size);

            std::for_each(std::execution::par_unseq,
                          indices.begin(), indices.end(),
                          [&](std::size_t j)
                          {
                              double dz = detail::activate_deriv(zptr[j], aptr[j], act);
                              delta[j] = dptr[j] * dz;
                          });

            // Gradients wrt bias and weights can be parallelized neuron-wise
            std::for_each(std::execution::par_unseq,
                          indices.begin(), indices.end(),
                          [&](std::size_t j)
                          {
                              double d = delta[j];
                              gbptr[j] += d;

                              double *row_gw = gwptr + j * in_size;
                              for (std::size_t i = 0; i < in_size; ++i)
                              {
                                  if (synaptic_pruning_mask[j * in_size + i])
                                  {
                                      row_gw[i] += d * inptr[i];
                                  }
                              }
                          });

            // dL_dinput = W^T * delta  (sequential to avoid data races)
            for (std::size_t j = 0; j < out_size; ++j)
            {
                const double d = delta[j];
                const double *row_w = wptr + j * in_size;
                for (std::size_t i = 0; i < in_size; ++i)
                {
                    if (synaptic_pruning_mask[j * in_size + i])
                    {
                        dinptr[i] += row_w[i] * d;
                    }
                }
            }
        }

        // Apply gradient step with plasticity mechanisms
        void apply_gradients(const std::vector<double> &grad_w,
                             const std::vector<double> &grad_b,
                             double lr,
                             const std::vector<double> &input,
                             const std::vector<double> &output)
        {
            assert(grad_w.size() == weights.size());
            assert(grad_b.size() == biases.size());
            assert(input.size() == in_size);
            assert(output.size() == out_size);

            // Update weights with gradient descent and Hebbian learning
            for (std::size_t j = 0; j < out_size; ++j)
            {
                for (std::size_t i = 0; i < in_size; ++i)
                {
                    if (synaptic_pruning_mask[j * in_size + i])
                    {
                        std::size_t idx = j * in_size + i;

                        // Standard backprop update
                        weights[idx] -= lr * grad_w[idx];

                        // Hebbian update (correlation-based plasticity)
                        weights[idx] += hebbian_learning_rate * input[i] * output[j] * plasticity_rates[idx];

                        // Decay eligibility traces
                        eligibility_traces[idx] *= decay_rate;

                        // Update eligibility trace based on current activity
                        eligibility_traces[idx] += input[i] * output[j];

                        // Homeostatic regulation - maintain target activation levels
                        double homeostatic_adjustment = homeostatic_strength *
                            (homeostatic_targets[j] - output[j]);
                        weights[idx] += homeostatic_adjustment;
                    }
                }
            }

            // Update biases
            std::transform(std::execution::par_unseq,
                           biases.begin(), biases.end(),
                           grad_b.begin(),
                           biases.begin(),
                           [lr](double b, double g)
                           { return b - lr * g; });
        }

        // Consolidate memories by strengthening important synapses
        void consolidate_memory(const std::vector<double>& importance_scores)
        {
            for (std::size_t j = 0; j < out_size; ++j)
            {
                for (std::size_t i = 0; i < in_size; ++i)
                {
                    std::size_t idx = j * in_size + i;
                    double importance = (i < importance_scores.size()) ? importance_scores[i] : 0.5;

                    // Strengthen important connections
                    if (importance > 0.7)
                    {
                        weights[idx] *= (1.0 + importance * 0.1);
                    }
                    // Or weaken less important connections
                    else if (importance < 0.3)
                    {
                        weights[idx] *= (1.0 - (1.0 - importance) * 0.05);
                    }

                    // Update homeostatic target based on consolidation
                    homeostatic_targets[j] = 0.5 + 0.3 * importance;
                }
            }
        }

        // Apply synaptic pruning to remove weak connections
        void prune_synapses()
        {
            for (std::size_t j = 0; j < out_size; ++j)
            {
                for (std::size_t i = 0; i < in_size; ++i)
                {
                    std::size_t idx = j * in_size + i;
                    if (std::abs(weights[idx]) < pruning_threshold)
                    {
                        synaptic_pruning_mask[idx] = false;
                        weights[idx] = 0.0; // Set to zero to ensure it's ignored
                    }
                }
            }
        }
    };

    class NeuralNetwork
    {
    private:
        struct DenseLayer
        {
            std::size_t in_size{};
            std::size_t out_size{};

            // weights[j * in_size + i] -> connection from i (prev) to j (this)
            std::vector<double> weights;
            std::vector<double> biases;

            // Caches for forward/backward
            std::vector<double> z_cache;      // pre-activation
            std::vector<double> a_cache;      // post-activation
            std::vector<std::size_t> indices; // for parallel loops

            DenseLayer() = default;

            DenseLayer(std::size_t in, std::size_t out, std::mt19937_64 &rng)
                : in_size(in),
                  out_size(out),
                  weights(in * out),
                  biases(out),
                  z_cache(out),
                  a_cache(out),
                  indices(out)
            {
                // Kaiming / He-like initialization (simple version)
                std::normal_distribution<double> dist(
                    0.0, std::sqrt(2.0 / static_cast<double>(in_size)));

                for (auto &w : weights)
                {
                    w = dist(rng);
                }
                std::fill(biases.begin(), biases.end(), 0.0);

                std::iota(indices.begin(), indices.end(), std::size_t{0});
            }

            // Forward into provided buffers (z_out, a_out)
            void forward(const std::vector<double> &input,
                         std::vector<double> &z_out,
                         std::vector<double> &a_out,
                         Activation act) const
            {
                assert(input.size() == in_size);
                assert(z_out.size() == out_size);
                assert(a_out.size() == out_size);

                const double *wptr = weights.data();
                const double *bptr = biases.data();
                const double *inptr = input.data();
                double *zptr = z_out.data();
                double *aptr = a_out.data();

                std::for_each(std::execution::par_unseq,
                              indices.begin(), indices.end(),
                              [&](std::size_t j)
                              {
                                  double sum = bptr[j];
                                  const double *row = wptr + j * in_size;
                                  for (std::size_t i = 0; i < in_size; ++i)
                                  {
                                      sum += row[i] * inptr[i];
                                  }
                                  zptr[j] = sum;
                                  aptr[j] = detail::activate(sum, act);
                              });
            }

            // Forward using internal caches
            void forward_cache(const std::vector<double> &input,
                               Activation act)
            {
                forward(input, z_cache, a_cache, act);
            }

            // Backward:
            // dL_dout  : derivative of loss wrt this layer's output (a_cache)
            // input    : previous layer activation (a_prev)
            // dL_dinput: output, gradient wrt input (same size as in_size)
            // grad_w   : gradient wrt weights (same size as weights)
            // grad_b   : gradient wrt biases (same size as biases)
            void backward(const std::vector<double> &input,
                          const std::vector<double> &dL_dout,
                          std::vector<double> &dL_dinput,
                          std::vector<double> &grad_w,
                          std::vector<double> &grad_b,
                          Activation act) const
            {
                assert(input.size() == in_size);
                assert(dL_dout.size() == out_size);
                assert(dL_dinput.size() == in_size);
                assert(grad_w.size() == weights.size());
                assert(grad_b.size() == biases.size());

                std::fill(dL_dinput.begin(), dL_dinput.end(), 0.0);
                std::fill(grad_w.begin(), grad_w.end(), 0.0);
                std::fill(grad_b.begin(), grad_b.end(), 0.0);

                const double *inptr = input.data();
                const double *dptr = dL_dout.data();
                const double *zptr = z_cache.data();
                const double *aptr = a_cache.data();

                double *gwptr = grad_w.data();
                double *gbptr = grad_b.data();
                double *dinptr = dL_dinput.data();
                const double *wptr = weights.data();

                // First, compute local delta for each neuron j
                std::vector<double> delta(out_size);

                std::for_each(std::execution::par_unseq,
                              indices.begin(), indices.end(),
                              [&](std::size_t j)
                              {
                                  double dz = detail::activate_deriv(zptr[j], aptr[j], act);
                                  delta[j] = dptr[j] * dz;
                              });

                // Gradients wrt bias and weights can be parallelized neuron-wise
                std::for_each(std::execution::par_unseq,
                              indices.begin(), indices.end(),
                              [&](std::size_t j)
                              {
                                  double d = delta[j];
                                  gbptr[j] += d;

                                  double *row_gw = gwptr + j * in_size;
                                  for (std::size_t i = 0; i < in_size; ++i)
                                  {
                                      row_gw[i] += d * inptr[i];
                                  }
                              });

                // dL_dinput = W^T * delta  (sequential to avoid data races)
                for (std::size_t j = 0; j < out_size; ++j)
                {
                    const double d = delta[j];
                    const double *row_w = wptr + j * in_size;
                    for (std::size_t i = 0; i < in_size; ++i)
                    {
                        dinptr[i] += row_w[i] * d;
                    }
                }
            }

            // Apply gradient step
            void apply_gradients(const std::vector<double> &grad_w,
                                 const std::vector<double> &grad_b,
                                 double lr)
            {
                assert(grad_w.size() == weights.size());
                assert(grad_b.size() == biases.size());

                std::transform(std::execution::par_unseq,
                               weights.begin(), weights.end(),
                               grad_w.begin(),
                               weights.begin(),
                               [lr](double w, double g)
                               { return w - lr * g; });

                std::transform(std::execution::par_unseq,
                               biases.begin(), biases.end(),
                               grad_b.begin(),
                               biases.begin(),
                               [lr](double b, double g)
                               { return b - lr * g; });
            }
        };

    public:
        NeuralNetwork() = default;

        NeuralNetwork(const std::vector<std::size_t> &layer_sizes,
                      Activation hidden_act = Activation::Relu,
                      Activation output_act = Activation::Linear)
            : hidden_activation_(hidden_act),
              output_activation_(output_act),
              use_plasticity_(true)  // Enable plasticity by default
        {
            assert(layer_sizes.size() >= 2);

            std::random_device rd;
            std::mt19937_64 rng(rd());

            for (std::size_t i = 0; i + 1 < layer_sizes.size(); ++i)
            {
                if (use_plasticity_) {
                    plastic_layers_.emplace_back(layer_sizes[i], layer_sizes[i + 1], rng);
                } else {
                    layers_.emplace_back(layer_sizes[i], layer_sizes[i + 1], rng);
                }
            }

            // Temporary buffers for forward pass
            forward_buffers_.resize(layer_sizes.size());
            for (std::size_t i = 0; i < layer_sizes.size(); ++i)
            {
                forward_buffers_[i].resize(layer_sizes[i]);
            }
        }

        void set_debug(bool enabled)
        {
            debug_enabled_ = enabled;
        }

        // Enable or disable synaptic plasticity
        void set_plasticity(bool enabled)
        {
            use_plasticity_ = enabled;
        }

        // Consolidate important memories in all layers
        void consolidate_memories(const std::vector<double>& importance_scores)
        {
            for (auto& layer : plastic_layers_)
            {
                layer.consolidate_memory(importance_scores);
            }
        }

        // Apply synaptic pruning to all layers
        void prune_synapses()
        {
            for (auto& layer : plastic_layers_)
            {
                layer.prune_synapses();
            }
        }

        std::size_t input_size() const
        {
            if (!plastic_layers_.empty())
                return plastic_layers_.front().in_size;
            else if (!layers_.empty())
                return layers_.front().in_size;
            else
                return 0;
        }

        std::size_t output_size() const
        {
            if (!plastic_layers_.empty())
                return plastic_layers_.back().out_size;
            else if (!layers_.empty())
                return layers_.back().out_size;
            else
                return 0;
        }

        // Forward inference
        std::vector<double> predict(const std::vector<double> &input) const
        {
            assert(input.size() == input_size());
            if (plastic_layers_.empty() && layers_.empty())
            {
                return {};
            }

            std::vector<double> a = input;
            std::vector<double> z;
            std::vector<double> next_a;

            if (!plastic_layers_.empty())
            {
                // Use plastic layers for prediction
                for (std::size_t idx = 0; idx < plastic_layers_.size(); ++idx)
                {
                    const auto &layer = plastic_layers_[idx];
                    Activation act = (idx + 1 == plastic_layers_.size())
                                         ? output_activation_
                                         : hidden_activation_;

                    z.resize(layer.out_size);
                    next_a.resize(layer.out_size);

                    layer.forward(a, z, next_a, act);
                    a = next_a;
                }
            }
            else
            {
                // Use standard layers for prediction
                z.resize(layers_[0].out_size);
                next_a.resize(layers_[0].out_size);

                for (std::size_t idx = 0; idx < layers_.size(); ++idx)
                {
                    const auto &layer = layers_[idx];
                    Activation act = (idx + 1 == layers_.size())
                                         ? output_activation_
                                         : hidden_activation_;

                    z.resize(layer.out_size);
                    next_a.resize(layer.out_size);

                    layer.forward(a, z, next_a, act);
                    a = next_a;
                }
            }
            return a;
        }

        // Simple SGD training with MSE loss and plasticity mechanisms
        void train(const std::vector<std::vector<double>> &X,
                   const std::vector<std::vector<double>> &Y,
                   std::size_t epochs,
                   std::size_t batch_size,
                   double learning_rate)
        {
            assert(X.size() == Y.size());
            if (X.empty() || (plastic_layers_.empty() && layers_.empty()))
                return;

            const std::size_t n = X.size();
            batch_size = std::max<std::size_t>(1, batch_size);

            if (!plastic_layers_.empty()) {
                // Training with plastic layers
                std::vector<std::vector<double>> grad_w(plastic_layers_.size());
                std::vector<std::vector<double>> grad_b(plastic_layers_.size());
                std::vector<std::vector<double>> dL_dinput(plastic_layers_.size());

                for (std::size_t l = 0; l < plastic_layers_.size(); ++l)
                {
                    grad_w[l].resize(plastic_layers_[l].weights.size());
                    grad_b[l].resize(plastic_layers_[l].biases.size());
                    dL_dinput[l].resize(plastic_layers_[l].in_size);
                }

                std::vector<double> dL_dout(output_size());

                for (std::size_t epoch = 0; epoch < epochs; ++epoch)
                {
                    double epoch_loss = 0.0;

                    // Reset gradients for epoch (we'll accumulate over mini-batches)
                    for (std::size_t l = 0; l < plastic_layers_.size(); ++l)
                    {
                        std::fill(grad_w[l].begin(), grad_w[l].end(), 0.0);
                        std::fill(grad_b[l].begin(), grad_b[l].end(), 0.0);
                    }

                    std::size_t batch_count = 0;

                    for (std::size_t start = 0; start < n; start += batch_size)
                    {
                        const std::size_t end = std::min(start + batch_size, n);
                        const std::size_t current_batch = end - start;

                        // Zero batch gradients
                        for (std::size_t l = 0; l < plastic_layers_.size(); ++l)
                        {
                            std::fill(grad_w[l].begin(), grad_w[l].end(), 0.0);
                            std::fill(grad_b[l].begin(), grad_b[l].end(), 0.0);
                        }

                        // Process each sample in batch (sequential over samples,
                        // parallel within layers)
                        for (std::size_t s = start; s < end; ++s)
                        {
                            const auto &x = X[s];
                            const auto &y = Y[s];
                            assert(x.size() == input_size());
                            assert(y.size() == output_size());

                            // Forward with caching
                            forward_buffers_[0] = x;
                            for (std::size_t l = 0; l < plastic_layers_.size(); ++l)
                            {
                                Activation act = (l + 1 == plastic_layers_.size())
                                                     ? output_activation_
                                                     : hidden_activation_;
                                plastic_layers_[l].forward_cache(forward_buffers_[l], act);
                                forward_buffers_[l + 1] = plastic_layers_[l].a_cache;
                            }

                            const auto &y_pred = forward_buffers_.back();

                            // MSE loss (0.5 * sum (y_pred - y)^2)
                            double sample_loss = 0.0;
                            for (std::size_t i = 0; i < y.size(); ++i)
                            {
                                const double diff = y_pred[i] - y[i];
                                sample_loss += diff * diff * 0.5;
                                dL_dout[i] = diff; // d/dy_pred (0.5*(diff^2)) = diff
                            }
                            epoch_loss += sample_loss;

                            // Backward pass
                            // Start from last layer
                            std::vector<double> next_dL = dL_dout;

                            for (std::size_t li = plastic_layers_.size(); li-- > 0;)
                            {
                                const auto &layer = plastic_layers_[li];
                                const auto &a_prev = forward_buffers_[li];

                                auto &layer_grad_w = grad_w[li];
                                auto &layer_grad_b = grad_b[li];
                                auto &layer_dL_dinput = dL_dinput[li];

                                Activation act = (li + 1 == plastic_layers_.size())
                                                     ? output_activation_
                                                     : hidden_activation_;

                                layer.backward(a_prev, next_dL,
                                               layer_dL_dinput,
                                               layer_grad_w,
                                               layer_grad_b,
                                               act);

                                next_dL = layer_dL_dinput;
                            }

                            // Accumulate gradients into batch gradients
                            // (already stored in grad_w/grad_b)
                        }

                        // Average batch gradients and apply updates
                        const double scale = 1.0 / static_cast<double>(current_batch);
                        for (std::size_t l = 0; l < plastic_layers_.size(); ++l)
                        {
                            auto &layer = plastic_layers_[l];

                            std::vector<double> scaled_w = grad_w[l];
                            std::vector<double> scaled_b = grad_b[l];

                            std::for_each(std::execution::par_unseq,
                                          scaled_w.begin(), scaled_w.end(),
                                          [scale](double &v)
                                          { v *= scale; });
                            std::for_each(std::execution::par_unseq,
                                          scaled_b.begin(), scaled_b.end(),
                                          [scale](double &v)
                                          { v *= scale; });

                            // Apply gradients with plasticity mechanisms
                            layer.apply_gradients(scaled_w, scaled_b, learning_rate,
                                                  forward_buffers_[l], forward_buffers_[l + 1]);
                        }

                        ++batch_count;
                    }

                    double avg_loss = epoch_loss / static_cast<double>(n);
                    log("Epoch " + std::to_string(epoch + 1) +
                        "/" + std::to_string(epochs) +
                        " - avg loss: " + std::to_string(avg_loss));
                }
            } else {
                // Standard training with regular layers (original implementation)
                std::vector<std::vector<double>> grad_w(layers_.size());
                std::vector<std::vector<double>> grad_b(layers_.size());
                std::vector<std::vector<double>> dL_dinput(layers_.size());

                for (std::size_t l = 0; l < layers_.size(); ++l)
                {
                    grad_w[l].resize(layers_[l].weights.size());
                    grad_b[l].resize(layers_[l].biases.size());
                    dL_dinput[l].resize(layers_[l].in_size);
                }

                std::vector<double> dL_dout(output_size());

                for (std::size_t epoch = 0; epoch < epochs; ++epoch)
                {
                    double epoch_loss = 0.0;

                    // Reset gradients for epoch (we'll accumulate over mini-batches)
                    for (std::size_t l = 0; l < layers_.size(); ++l)
                    {
                        std::fill(grad_w[l].begin(), grad_w[l].end(), 0.0);
                        std::fill(grad_b[l].begin(), grad_b[l].end(), 0.0);
                    }

                    std::size_t batch_count = 0;

                    for (std::size_t start = 0; start < n; start += batch_size)
                    {
                        const std::size_t end = std::min(start + batch_size, n);
                        const std::size_t current_batch = end - start;

                        // Zero batch gradients
                        for (std::size_t l = 0; l < layers_.size(); ++l)
                        {
                            std::fill(grad_w[l].begin(), grad_w[l].end(), 0.0);
                            std::fill(grad_b[l].begin(), grad_b[l].end(), 0.0);
                        }

                        // Process each sample in batch (sequential over samples,
                        // parallel within layers)
                        for (std::size_t s = start; s < end; ++s)
                        {
                            const auto &x = X[s];
                            const auto &y = Y[s];
                            assert(x.size() == input_size());
                            assert(y.size() == output_size());

                            // Forward with caching
                            forward_buffers_[0] = x;
                            for (std::size_t l = 0; l < layers_.size(); ++l)
                            {
                                Activation act = (l + 1 == layers_.size())
                                                     ? output_activation_
                                                     : hidden_activation_;
                                layers_[l].forward_cache(forward_buffers_[l], act);
                                forward_buffers_[l + 1] = layers_[l].a_cache;
                            }

                            const auto &y_pred = forward_buffers_.back();

                            // MSE loss (0.5 * sum (y_pred - y)^2)
                            double sample_loss = 0.0;
                            for (std::size_t i = 0; i < y.size(); ++i)
                            {
                                const double diff = y_pred[i] - y[i];
                                sample_loss += diff * diff * 0.5;
                                dL_dout[i] = diff; // d/dy_pred (0.5*(diff^2)) = diff
                            }
                            epoch_loss += sample_loss;

                            // Backward pass
                            // Start from last layer
                            std::vector<double> next_dL = dL_dout;

                            for (std::size_t li = layers_.size(); li-- > 0;)
                            {
                                const auto &layer = layers_[li];
                                const auto &a_prev = forward_buffers_[li];

                                auto &layer_grad_w = grad_w[li];
                                auto &layer_grad_b = grad_b[li];
                                auto &layer_dL_dinput = dL_dinput[li];

                                Activation act = (li + 1 == layers_.size())
                                                     ? output_activation_
                                                     : hidden_activation_;

                                layer.backward(a_prev, next_dL,
                                               layer_dL_dinput,
                                               layer_grad_w,
                                               layer_grad_b,
                                               act);

                                next_dL = layer_dL_dinput;
                            }

                            // Accumulate gradients into batch gradients
                            // (already stored in grad_w/grad_b)
                        }

                        // Average batch gradients and apply updates
                        const double scale = 1.0 / static_cast<double>(current_batch);
                        for (std::size_t l = 0; l < layers_.size(); ++l)
                        {
                            auto &layer = layers_[l];

                            std::vector<double> scaled_w = grad_w[l];
                            std::vector<double> scaled_b = grad_b[l];

                            std::for_each(std::execution::par_unseq,
                                          scaled_w.begin(), scaled_w.end(),
                                          [scale](double &v)
                                          { v *= scale; });
                            std::for_each(std::execution::par_unseq,
                                          scaled_b.begin(), scaled_b.end(),
                                          [scale](double &v)
                                          { v *= scale; });

                            layer.apply_gradients(scaled_w, scaled_b, learning_rate);
                        }

                        ++batch_count;
                    }

                    double avg_loss = epoch_loss / static_cast<double>(n);
                    log("Epoch " + std::to_string(epoch + 1) +
                        "/" + std::to_string(epochs) +
                        " - avg loss: " + std::to_string(avg_loss));
                }
            }
        }

    private:
        std::vector<DenseLayer> layers_;
        std::vector<PlasticLayer> plastic_layers_;
        Activation hidden_activation_{Activation::Relu};
        Activation output_activation_{Activation::Linear};

        // forward_buffers_[l] holds activations at layer l
        // l=0 -> input, l=end -> output
        std::vector<std::vector<double>> forward_buffers_;

        bool debug_enabled_{false};
        bool use_plasticity_{true};

        void log(const std::string &msg) const
        {
#ifdef DNN_ENABLE_DEBUG_LOG
            if (!debug_enabled_)
                return;
            std::lock_guard<std::mutex> lock(Logger::mutex());
            std::cerr << "[DNN] " << msg << '\n';
#else
            (void)msg;
#endif
        }
    };

} // namespace dnn
