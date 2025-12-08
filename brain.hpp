#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "dnn.hpp"

namespace brain
{
    using Tensor = std::vector<double>;

    struct BrainIO
    {
        const Tensor *sensory_input{nullptr};
        const Tensor *context_input{nullptr};
    };

    struct BrainOutput
    {
        Tensor output;
        Tensor context_out;
    };

    struct Decision
    {
        int action{-1};
        Tensor logits;
        Tensor probs;
        double value{0.0};
    };

    inline std::size_t param_count_from_layers(const std::vector<std::size_t> &sizes)
    {
        if (sizes.size() < 2)
            return 0;
        std::size_t total = 0;
        for (std::size_t i = 0; i + 1 < sizes.size(); ++i)
        {
            total += sizes[i] * sizes[i + 1]; // weights
            total += sizes[i + 1];            // biases
        }
        return total;
    }

    class BrainModule
    {
    public:
        virtual ~BrainModule() = default;
        virtual BrainOutput step(const BrainIO &in, double dt) = 0;
        virtual std::string name() const = 0;
        virtual std::size_t param_count() const = 0;
    };

    inline Tensor concat_inputs(const Tensor *a, const Tensor *b)
    {
        Tensor out;
        if (a)
            out.insert(out.end(), a->begin(), a->end());
        if (b)
            out.insert(out.end(), b->begin(), b->end());
        return out;
    }

    inline void fit_to_size(Tensor &v, std::size_t wanted)
    {
        if (v.size() < wanted)
        {
            v.resize(wanted, 0.0);
        }
        else if (v.size() > wanted)
        {
            v.resize(wanted);
        }
    }

    inline int argmax(const Tensor &v)
    {
        if (v.empty())
            return -1;
        return static_cast<int>(std::distance(
            v.begin(), std::max_element(v.begin(), v.end())));
    }

    inline Tensor softmax(const Tensor &logits, double temperature = 1.0)
    {
        Tensor probs(logits.size(), 0.0);
        if (logits.empty())
            return probs;
        const double inv_temp = 1.0 / std::max(temperature, 1e-6);
        const double max_logit = *std::max_element(logits.begin(), logits.end());

        double sum = 0.0;
        for (std::size_t i = 0; i < logits.size(); ++i)
        {
            const double z = (logits[i] - max_logit) * inv_temp;
            const double e = std::exp(z);
            probs[i] = e;
            sum += e;
        }

        if (sum > 0.0)
        {
            for (double &p : probs)
            {
                p /= sum;
            }
        }
        else
        {
            const double uniform = 1.0 / static_cast<double>(probs.size());
            std::fill(probs.begin(), probs.end(), uniform);
        }
        return probs;
    }

    inline int sample_from_probs(const Tensor &probs, std::mt19937_64 &rng)
    {
        if (probs.empty())
            return -1;
        std::discrete_distribution<int> dist(probs.begin(), probs.end());
        return dist(rng);
    }

    class VisionModule : public BrainModule
    {
    public:
        VisionModule(std::string name,
                     const std::vector<std::size_t> &layer_sizes,
                     dnn::Activation hidden_act = dnn::Activation::Relu,
                     dnn::Activation output_act = dnn::Activation::Relu)
            : name_(std::move(name)),
              layer_sizes_(layer_sizes),
              net_(layer_sizes, hidden_act, output_act)
        {
        }

        BrainOutput step(const BrainIO &in, double /*dt*/) override
        {
            if (!in.sensory_input)
                return {};

            Tensor x = concat_inputs(in.sensory_input, in.context_input);
            fit_to_size(x, layer_sizes_.front());

            Tensor feat = net_.predict(x);
            return {feat, feat}; // expose features to the global context
        }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers(layer_sizes_); }

    private:
        std::string name_;
        std::vector<std::size_t> layer_sizes_;
        dnn::NeuralNetwork net_;
    };

    class MemoryModule : public BrainModule
    {
    public:
        MemoryModule(std::string name,
                     std::size_t sensory_size,
                     std::size_t context_size,
                     std::size_t hidden_size,
                     double leak = 0.0,
                     dnn::Activation hidden_act = dnn::Activation::Tanh)
            : name_(std::move(name)),
              hidden_size_(hidden_size),
              leak_(leak),
              layer_sizes_{sensory_size + context_size + hidden_size, hidden_size},
              net_(layer_sizes_, hidden_act, dnn::Activation::Tanh),
              hidden_state_(hidden_size, 0.0)
        {
        }

        BrainOutput step(const BrainIO &in, double dt) override
        {
            Tensor x = concat_inputs(in.sensory_input, in.context_input);
            x.insert(x.end(), hidden_state_.begin(), hidden_state_.end());
            fit_to_size(x, layer_sizes_.front());

            if (leak_ > 0.0)
            {
                const double decay = std::clamp(1.0 - leak_ * dt, 0.0, 1.0);
                for (double &h : hidden_state_)
                {
                    h *= decay;
                }
            }

            Tensor next = net_.predict(x);
            fit_to_size(next, hidden_size_);
            hidden_state_ = next;
            return {hidden_state_, hidden_state_};
        }

        void reset_state(double value = 0.0)
        {
            std::fill(hidden_state_.begin(), hidden_state_.end(), value);
        }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers(layer_sizes_); }

    private:
        std::string name_;
        std::size_t hidden_size_{0};
        double leak_{0.0};
        std::vector<std::size_t> layer_sizes_;
        dnn::NeuralNetwork net_;
        Tensor hidden_state_;
    };

    class AttentionModule : public BrainModule
    {
    public:
        AttentionModule(std::string name,
                        std::size_t sensory_size,
                        std::size_t context_size,
                        std::size_t hidden_size)
            : name_(std::move(name)),
              layer_sizes_{sensory_size + context_size, hidden_size, context_size},
              net_(layer_sizes_, dnn::Activation::Relu, dnn::Activation::Tanh)
        {
        }

        BrainOutput step(const BrainIO &in, double /*dt*/) override
        {
            Tensor x = concat_inputs(in.sensory_input, in.context_input);
            fit_to_size(x, layer_sizes_.front());

            Tensor attended = net_.predict(x);
            return {attended, attended};
        }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers(layer_sizes_); }

    private:
        std::string name_;
        std::vector<std::size_t> layer_sizes_;
        dnn::NeuralNetwork net_;
    };

    class WorldModelModule : public BrainModule
    {
    public:
        WorldModelModule(std::string name,
                         std::size_t sensory_size,
                         std::size_t context_size,
                         std::size_t hidden_size)
            : name_(std::move(name)),
              sensory_size_(sensory_size),
              layer_sizes_{sensory_size + context_size, hidden_size, sensory_size},
              net_(layer_sizes_, dnn::Activation::Relu, dnn::Activation::Linear),
              last_prediction_(sensory_size, 0.0)
        {
        }

        BrainOutput step(const BrainIO &in, double /*dt*/) override
        {
            Tensor x = concat_inputs(in.sensory_input, in.context_input);
            fit_to_size(x, layer_sizes_.front());

            last_prediction_ = net_.predict(x);
            fit_to_size(last_prediction_, sensory_size_);
            return {last_prediction_, last_prediction_};
        }

        const Tensor &last_prediction() const { return last_prediction_; }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers(layer_sizes_); }

    private:
        std::string name_;
        std::size_t sensory_size_{0};
        std::vector<std::size_t> layer_sizes_;
        dnn::NeuralNetwork net_;
        Tensor last_prediction_;
    };

    class ValueModule : public BrainModule
    {
    public:
        ValueModule(std::string name,
                    std::size_t sensory_size,
                    std::size_t context_size,
                    std::size_t hidden_size)
            : name_(std::move(name)),
              layer_sizes_{sensory_size + context_size, hidden_size, 1},
              net_(layer_sizes_, dnn::Activation::Relu, dnn::Activation::Linear)
        {
        }

        BrainOutput step(const BrainIO &in, double /*dt*/) override
        {
            Tensor x = concat_inputs(in.sensory_input, in.context_input);
            fit_to_size(x, layer_sizes_.front());
            Tensor v = net_.predict(x);
            return {v, {}};
        }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers(layer_sizes_); }

    private:
        std::string name_;
        std::vector<std::size_t> layer_sizes_;
        dnn::NeuralNetwork net_;
    };

    class PolicyModule : public BrainModule
    {
    public:
        PolicyModule(std::string name,
                     const std::vector<std::size_t> &layer_sizes,
                     dnn::Activation hidden_act = dnn::Activation::Relu,
                     dnn::Activation output_act = dnn::Activation::Linear)
            : name_(std::move(name)),
              layer_sizes_(layer_sizes),
              net_(layer_sizes, hidden_act, output_act)
        {
        }

        BrainOutput step(const BrainIO &in, double /*dt*/) override
        {
            Tensor x = concat_inputs(in.sensory_input, in.context_input);
            fit_to_size(x, layer_sizes_.front());
            Tensor logits = net_.predict(x);
            return {logits, {}};
        }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers(layer_sizes_); }

    private:
        std::string name_;
        std::vector<std::size_t> layer_sizes_;
        dnn::NeuralNetwork net_;
    };

    class BrainEngine
    {
    public:
        struct ModuleEntry
        {
            std::unique_ptr<BrainModule> module;
            bool uses_sensory{true};
            bool uses_context{true};
            int steps_per_call{1};
            int step_counter{0};
        };

        int add_module(std::unique_ptr<BrainModule> m,
                       bool uses_sensory,
                       bool uses_context,
                       int steps_per_call = 1)
        {
            ModuleEntry e;
            e.module = std::move(m);
            e.uses_sensory = uses_sensory;
            e.uses_context = uses_context;
            e.steps_per_call = std::max(1, steps_per_call);
            e.step_counter = 0;
            modules_.push_back(std::move(e));
            return static_cast<int>(modules_.size() - 1);
        }

        void set_dt(double dt) { dt_ = dt; }
        void set_settling_steps(int n) { settling_steps_ = std::max(1, n); }
        void set_context_blend(double mix) { context_blend_ = std::clamp(mix, 0.0, 1.0); }
        void set_context_clip(double clip) { context_clip_ = std::max(clip, 0.0); }

        void set_context_size(std::size_t n)
        {
            context_.assign(n, 0.0);
        }

        const Tensor &context() const { return context_; }

        void reset()
        {
            std::fill(context_.begin(), context_.end(), 0.0);
            for (auto &m : modules_)
            {
                m.step_counter = 0;
            }
        }

        Tensor step(const Tensor &env_obs, int policy_module_index = -1)
        {
            Tensor policy_out;
            bool have_policy = false;

            for (int iter = 0; iter < settling_steps_; ++iter)
            {
                Tensor aggregate = context_;
                std::size_t contributors = aggregate.empty() ? 0 : 1;

                for (std::size_t idx = 0; idx < modules_.size(); ++idx)
                {
                    auto &entry = modules_[idx];
                    const bool run_now = (entry.step_counter++ % entry.steps_per_call) == 0;
                    if (!run_now)
                        continue;

                    BrainIO io{
                        entry.uses_sensory ? &env_obs : nullptr,
                        entry.uses_context ? &context_ : nullptr};

                    BrainOutput out = entry.module->step(io, dt_);

                    if (!out.context_out.empty())
                    {
                        if (aggregate.size() < out.context_out.size())
                            aggregate.resize(out.context_out.size(), 0.0);
                        for (std::size_t i = 0; i < out.context_out.size(); ++i)
                        {
                            aggregate[i] += out.context_out[i];
                        }
                        ++contributors;
                    }

                    if (static_cast<int>(idx) == policy_module_index)
                    {
                        policy_out = out.output;
                        have_policy = true;
                    }
                }

                if (contributors > 0 && !aggregate.empty())
                {
                    const double scale = 1.0 / static_cast<double>(contributors);
                    for (double &v : aggregate)
                    {
                        v *= scale;
                    }
                }

                if (!aggregate.empty())
                {
                    if (context_.size() < aggregate.size())
                        context_.resize(aggregate.size(), 0.0);

                    const double blend = context_blend_;
                    for (std::size_t i = 0; i < aggregate.size(); ++i)
                    {
                        double mixed = aggregate[i];
                        if (blend < 1.0)
                        {
                            mixed = context_[i] * (1.0 - blend) + aggregate[i] * blend;
                        }
                        if (context_clip_ > 0.0)
                        {
                            mixed = std::clamp(mixed, -context_clip_, context_clip_);
                        }
                        context_[i] = mixed;
                    }
                }
            }

            if (have_policy)
                return policy_out;
            return context_;
        }

    private:
        std::vector<ModuleEntry> modules_;
        Tensor context_;
        double dt_{1.0};
        int settling_steps_{1};
        double context_blend_{1.0};
        double context_clip_{5.0};
    };

    class CognitiveBrain
    {
    public:
        CognitiveBrain(std::size_t sensory_size,
                       std::size_t action_count,
                       std::size_t context_size = 64)
            : sensory_size_(sensory_size),
              action_count_(action_count),
              augmented_obs_(sensory_size + 1, 0.0)
        {
            engine_.set_context_size(context_size);
            engine_.set_dt(0.05);
            engine_.set_settling_steps(2);
            engine_.set_context_blend(0.65);
            engine_.set_context_clip(3.0);

            auto vision = std::make_unique<VisionModule>(
                "vision",
                std::vector<std::size_t>{sensory_size_ + 1, context_size},
                dnn::Activation::Relu,
                dnn::Activation::Tanh);

            auto attention = std::make_unique<AttentionModule>(
                "attention", sensory_size_ + 1, context_size, context_size);

            auto working = std::make_unique<MemoryModule>(
                "working-memory", sensory_size_ + 1, context_size, context_size, 0.01);

            auto world = std::make_unique<WorldModelModule>(
                "world-model", sensory_size_ + 1, context_size, context_size);

            auto policy = std::make_unique<PolicyModule>(
                "policy",
                std::vector<std::size_t>{context_size + context_size, context_size, static_cast<std::size_t>(action_count_)},
                dnn::Activation::Relu,
                dnn::Activation::Linear);

            auto value = std::make_unique<ValueModule>(
                "value", sensory_size_ + 1, context_size, context_size);
            value_module_ = value.get();

            engine_.add_module(std::move(vision), true, false);
            engine_.add_module(std::move(attention), true, true);
            engine_.add_module(std::move(working), true, true);
            engine_.add_module(std::move(world), true, true, 2);
            policy_idx_ = engine_.add_module(std::move(policy), false, true);
            engine_.add_module(std::move(value), false, true);
        }

        Tensor act(const Tensor &observation, double reward)
        {
            if (observation.size() != sensory_size_)
                augmented_obs_.assign(sensory_size_ + 1, 0.0);
            std::copy(observation.begin(), observation.end(), augmented_obs_.begin());
            augmented_obs_.back() = reward;

            Tensor logits = engine_.step(augmented_obs_, policy_idx_);

            if (value_module_)
            {
                BrainIO io{&augmented_obs_, &engine_.context()};
                auto v = value_module_->step(io, 0.0).output;
                last_value_ = v.empty() ? 0.0 : v.front();
            }

            return logits;
        }

        Decision decide(const Tensor &observation,
                        double reward,
                        double temperature = 1.0,
                        bool greedy = false)
        {
            Tensor logits = act(observation, reward);
            Tensor probs = softmax(logits, temperature);
            int action = greedy ? argmax(probs) : sample_from_probs(probs, rng_);
            return Decision{action, std::move(logits), std::move(probs), value_estimate()};
        }

        const Tensor &context() const { return engine_.context(); }
        double value_estimate() const { return last_value_; }
        void set_seed(std::uint64_t seed) { rng_.seed(seed); }

    private:
        BrainEngine engine_;
        std::size_t sensory_size_{0};
        std::size_t action_count_{0};
        Tensor augmented_obs_;
        int policy_idx_{-1};
        ValueModule *value_module_{nullptr};
        double last_value_{0.0};
        std::mt19937_64 rng_{std::random_device{}()};
    };

} // namespace brain
