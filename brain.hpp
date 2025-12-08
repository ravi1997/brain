#pragma once

#include <algorithm>
#include <memory>
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
            Tensor aggregate = context_;
            std::size_t contributors = aggregate.empty() ? 0 : 1;
            Tensor policy_out;
            bool have_policy = false;

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
                context_ = std::move(aggregate);

            if (have_policy)
                return policy_out;
            return context_;
        }

    private:
        std::vector<ModuleEntry> modules_;
        Tensor context_;
        double dt_{1.0};
    };

} // namespace brain
