#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "dnn.hpp"

namespace brain
{
    using Tensor = std::vector<double>;

    // Learning phase states for dynamic input processing
    enum class LearningPhase
    {
        ACQUISITION,    // Initial learning phase
        CONSOLIDATION,  // Memory consolidation phase
        RETRIEVAL,      // Knowledge retrieval phase
        TESTING         // Evaluation/test phase
    };

    // Knowledge hierarchy structure for progressive learning
    struct KnowledgeNode
    {
        std::string concept_name;
        Tensor representation;
        std::vector<std::string> related_concepts;
        double confidence{1.0};  // How confident we are in this knowledge
        std::chrono::steady_clock::time_point creation_time;
        std::chrono::steady_clock::time_point last_accessed;
        int access_count{1};

        KnowledgeNode() : creation_time(std::chrono::steady_clock::now()),
                         last_accessed(std::chrono::steady_clock::now()) {}
    };

    // Information about input processing and its phase
    struct InputProcessingInfo
    {
        LearningPhase current_phase{LearningPhase::ACQUISITION};
        double phase_confidence{1.0};
        std::string input_text;
        Tensor processed_tensor;
        std::vector<std::string> extracted_concepts;
        double novelty_score{0.0};  // How novel this input is
        bool is_conflicting{false}; // Whether this input conflicts with existing knowledge
        std::string conflict_details;
        std::chrono::steady_clock::time_point timestamp;

        InputProcessingInfo() : timestamp(std::chrono::steady_clock::now()) {}
    };

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

    struct Experience
    {
        Tensor obs_with_reward; // sensory + reward channel
        Tensor context_before;
        Tensor next_obs;
        double reward{0.0};
        int action{-1};
    };

    // Enhanced experience for sophisticated learning
    struct EnhancedExperience
    {
        Tensor observation;
        Tensor context_before;
        Tensor context_after;
        Tensor next_obs;  // Added to match Experience struct
        Tensor action_taken;
        double reward{0.0};
        double expected_reward{0.0};
        double prediction_error{0.0};
        LearningPhase phase{LearningPhase::ACQUISITION};
        std::vector<std::string> related_concepts;
        std::chrono::steady_clock::time_point timestamp;
        double importance{1.0};  // How important this experience is

        EnhancedExperience() : timestamp(std::chrono::steady_clock::now()) {}
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

    [[nodiscard]] inline Tensor concat_inputs(const Tensor *a, const Tensor *b)
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

    [[nodiscard]] inline int argmax(const Tensor &v)
    {
        if (v.empty())
            return -1;
        return static_cast<int>(std::distance(
            v.begin(), std::max_element(v.begin(), v.end())));
    }

    inline Tensor concat(const Tensor &a, const Tensor &b)
    {
        Tensor out;
        out.reserve(a.size() + b.size());
        out.insert(out.end(), a.begin(), a.end());
        out.insert(out.end(), b.begin(), b.end());
        return out;
    }

    [[nodiscard]] inline Tensor softmax(const Tensor &logits, double temperature = 1.0)
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
                         std::size_t hidden_size,
                         bool include_reward = true)
            : name_(std::move(name)),
              predict_size_(sensory_size),
              input_size_(sensory_size + context_size + (include_reward ? 1 : 0)),
              layer_sizes_{input_size_, hidden_size, predict_size_},
              net_(layer_sizes_, dnn::Activation::Relu, dnn::Activation::Linear),
              last_prediction_(predict_size_, 0.0)
        {
        }

        BrainOutput step(const BrainIO &in, double /*dt*/) override
        {
            Tensor x = concat_inputs(in.sensory_input, in.context_input);
            fit_to_size(x, layer_sizes_.front());

            last_prediction_ = net_.predict(x);
            fit_to_size(last_prediction_, predict_size_);
            return {last_prediction_, last_prediction_};
        }

        const Tensor &last_prediction() const { return last_prediction_; }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers(layer_sizes_); }

        void train(const std::vector<Tensor> &X,
                   const std::vector<Tensor> &Y,
                   std::size_t epochs,
                   std::size_t batch,
                   double lr)
        {
            net_.train(X, Y, epochs, batch, lr);
        }

    private:
        std::string name_;
        std::size_t predict_size_{0};
        std::size_t input_size_{0};
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

        void train(const std::vector<Tensor> &X,
                   const std::vector<Tensor> &Y,
                   std::size_t epochs,
                   std::size_t batch,
                   double lr)
        {
            net_.train(X, Y, epochs, batch, lr);
        }

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
              context_size_(context_size),
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
                "world-model", sensory_size_, context_size, context_size, true);
            world_module_ = world.get();

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
            last_context_before_ = engine_.context();
            if (observation.size() != sensory_size_)
                augmented_obs_.assign(sensory_size_ + 1, 0.0);
            std::copy(observation.begin(), observation.end(), augmented_obs_.begin());
            augmented_obs_.back() = reward;

            Tensor logits = engine_.step(augmented_obs_, policy_idx_);
            last_reward_ = reward;
            last_aug_obs_ = augmented_obs_;

            if (value_module_)
            {
                BrainIO io{&augmented_obs_, &engine_.context()};
                auto v = value_module_->step(io, 0.0).output;
                last_value_ = v.empty() ? 0.0 : v.front();
            }

            return logits;
        }

        [[nodiscard]] Decision decide(const Tensor &observation,
                                     double reward,
                                     double temperature = 1.0,
                                     bool greedy = false)
        {
            Tensor logits = act(observation, reward);
            Tensor probs = softmax(logits, temperature);
            int action = greedy ? argmax(probs) : sample_from_probs(probs, rng_);
            last_decision_ = Decision{action, logits, probs, value_estimate()};
            return last_decision_;
        }

        void record_transition(const Tensor &next_observation)
        {
            if (last_decision_.action < 0)
                return;
            Experience e;
            e.obs_with_reward = last_aug_obs_;
            e.context_before = last_context_before_;
            e.next_obs = next_observation;
            e.reward = last_reward_;
            e.action = last_decision_.action;

            if (experiences_.size() >= max_experiences_)
            {
                experiences_.erase(experiences_.begin());
            }
            experiences_.push_back(std::move(e));
        }

        const Tensor &context() const { return engine_.context(); }
        double value_estimate() const { return last_value_; }
        void set_seed(std::uint64_t seed) { rng_.seed(seed); }

        void set_experience_limit(std::size_t n) { max_experiences_ = std::max<std::size_t>(8, n); }

        void learn_from_experience(std::size_t epochs = 3,
                                   std::size_t batch = 8,
                                   double lr_value = 0.01,
                                   double lr_world = 0.005,
                                   double gamma = 0.95)
        {
            if (experiences_.empty() || !value_module_ || !world_module_)
                return;

            std::vector<double> returns(experiences_.size(), 0.0);
            double running = 0.0;
            for (std::size_t i = experiences_.size(); i-- > 0;)
            {
                running = experiences_[i].reward + gamma * running;
                returns[i] = running;
            }

            std::vector<Tensor> vx;
            std::vector<Tensor> vy;
            std::vector<Tensor> wx;
            std::vector<Tensor> wy;

            vx.reserve(experiences_.size());
            vy.reserve(experiences_.size());
            wx.reserve(experiences_.size());
            wy.reserve(experiences_.size());

            for (std::size_t i = 0; i < experiences_.size(); ++i)
            {
                Tensor input = concat(experiences_[i].obs_with_reward, experiences_[i].context_before);
                fit_to_size(input, sensory_size_ + 1 + context_size_);
                vx.push_back(input);
                vy.push_back(Tensor{returns[i]});

                Tensor w_in = input;
                Tensor target = experiences_[i].next_obs;
                fit_to_size(target, sensory_size_);
                wx.push_back(std::move(w_in));
                wy.push_back(std::move(target));
            }

            value_module_->train(vx, vy, epochs, batch, lr_value);
            world_module_->train(wx, wy, epochs, batch, lr_world);
        }

    private:
        BrainEngine engine_;
        std::size_t sensory_size_{0};
        std::size_t action_count_{0};
        std::size_t context_size_{0};
        Tensor augmented_obs_;
        int policy_idx_{-1};
        ValueModule *value_module_{nullptr};
        WorldModelModule *world_module_{nullptr};
        double last_value_{0.0};
        Decision last_decision_;
        std::mt19937_64 rng_{std::random_device{}()};
        Tensor last_aug_obs_;
        Tensor last_context_before_;
        double last_reward_{0.0};
        std::vector<Experience> experiences_;
        std::size_t max_experiences_{512};
    };

    // Multi-layered cognitive processing modules
    class SensoryProcessingModule : public BrainModule
    {
    public:
        SensoryProcessingModule(std::string name,
                               std::size_t input_size,
                               std::size_t output_size,
                               dnn::Activation activation = dnn::Activation::Relu)
            : name_(std::move(name)),
              input_size_(input_size),
              output_size_(output_size),
              net_({input_size, (input_size + output_size) / 2, output_size}, activation, dnn::Activation::Tanh)
        {
        }

        BrainOutput step(const BrainIO &in, double /*dt*/) override
        {
            if (!in.sensory_input)
                return {};

            Tensor input = concat_inputs(in.sensory_input, in.context_input);
            if (input.size() != input_size_)
                fit_to_size(input, input_size_);

            Tensor features = net_.predict(input);
            return {features, features};
        }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers({input_size_, (input_size_ + output_size_) / 2, output_size_}); }

    private:
        std::string name_;
        std::size_t input_size_;
        std::size_t output_size_;
        dnn::NeuralNetwork net_;
    };

    class MemoryConsolidationModule : public BrainModule
    {
    public:
        MemoryConsolidationModule(std::string name,
                                 std::size_t input_size,
                                 std::size_t context_size,
                                 std::size_t hidden_size,
                                 double leak_rate = 0.01)
            : name_(std::move(name)),
              hidden_size_(hidden_size),
              leak_rate_(leak_rate),
              net_({input_size + context_size, hidden_size, context_size}, dnn::Activation::Tanh, dnn::Activation::Tanh),
              state_(context_size, 0.0)
        {
        }

        BrainOutput step(const BrainIO &in, double dt) override
        {
            if (!in.sensory_input && !in.context_input)
                return {};

            Tensor input = concat_inputs(in.sensory_input, in.context_input);
            fit_to_size(input, net_.input_size());

            // Apply leak to state
            if (leak_rate_ > 0.0) {
                double decay = std::exp(-leak_rate_ * dt);
                for (double &s : state_) {
                    s *= decay;
                }
            }

            Tensor output = net_.predict(input);

            // Update state with new output
            for (size_t i = 0; i < std::min(state_.size(), output.size()); ++i) {
                state_[i] = output[i];
            }

            return {output, state_};
        }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers({net_.input_size(), net_.output_size()}); }

        const Tensor& get_state() const { return state_; }
        void reset_state() { std::fill(state_.begin(), state_.end(), 0.0); }

    private:
        std::string name_;
        std::size_t hidden_size_;
        double leak_rate_;
        dnn::NeuralNetwork net_;
        Tensor state_;
    };

    class PatternRecognitionModule : public BrainModule
    {
    public:
        PatternRecognitionModule(std::string name,
                                std::size_t input_size,
                                std::size_t pattern_size,
                                std::size_t num_patterns)
            : name_(std::move(name)),
              input_size_(input_size),
              pattern_size_(pattern_size),
              num_patterns_(num_patterns),
              pattern_net_({input_size, (input_size + pattern_size * num_patterns) / 2, pattern_size * num_patterns}, dnn::Activation::Relu, dnn::Activation::Tanh),
              pattern_weights_(pattern_size * num_patterns, 0.0),
              pattern_activations_(num_patterns, 0.0)
        {
        }

        BrainOutput step(const BrainIO &in, double /*dt*/) override
        {
            if (!in.sensory_input && !in.context_input)
                return {};

            Tensor input = concat_inputs(in.sensory_input, in.context_input);
            fit_to_size(input, input_size_);

            // Get pattern representations from network
            Tensor patterns = pattern_net_.predict(input);
            fit_to_size(patterns, pattern_size_ * num_patterns_);

            // Calculate similarity with stored patterns
            std::fill(pattern_activations_.begin(), pattern_activations_.end(), 0.0);

            for (size_t p = 0; p < num_patterns_; ++p) {
                double sim = 0.0;
                double norm1 = 1e-9, norm2 = 1e-9;

                for (size_t i = 0; i < pattern_size_; ++i) {
                    double val1 = patterns[p * pattern_size_ + i];
                    double val2 = pattern_weights_[p * pattern_size_ + i];
                    sim += val1 * val2;
                    norm1 += val1 * val1;
                    norm2 += val2 * val2;
                }

                pattern_activations_[p] = sim / std::sqrt(norm1 * norm2);
            }

            // Return top activation
            int max_idx = argmax(pattern_activations_);
            Tensor result(pattern_size_, 0.0);
            for (size_t i = 0; i < pattern_size_; ++i) {
                result[i] = patterns[max_idx * pattern_size_ + i];
            }

            return {result, pattern_activations_};
        }

        std::string name() const override { return name_; }
        std::size_t param_count() const override { return param_count_from_layers({input_size_, (input_size_ + pattern_size_ * num_patterns_) / 2, pattern_size_ * num_patterns_}); }

    private:
        std::string name_;
        std::size_t input_size_;
        std::size_t pattern_size_;
        std::size_t num_patterns_;
        dnn::NeuralNetwork pattern_net_;
        Tensor pattern_weights_;
        Tensor pattern_activations_;
    };

    class KnowledgeAbstractionModule : public BrainModule
    {
    public:
        KnowledgeAbstractionModule(std::string name,
                                  std::size_t input_size,
                                  std::size_t output_size,
                                  std::size_t abstraction_levels = 3)
            : name_(std::move(name)),
              input_size_(input_size),
              output_size_(output_size),
              abstraction_levels_(abstraction_levels)
        {
            // Create hierarchical abstraction networks
            for (size_t level = 0; level < abstraction_levels_; ++level) {
                size_t layer_input = (level == 0) ? input_size_ : output_size_;
                abstraction_nets_.emplace_back(
                    std::vector<std::size_t>{layer_input, (layer_input + output_size_) / 2, output_size_},
                    dnn::Activation::Relu,
                    dnn::Activation::Tanh
                );
            }
            abstraction_cache_.resize(abstraction_levels_);
        }

        BrainOutput step(const BrainIO &in, double /*dt*/) override
        {
            if (!in.sensory_input && !in.context_input)
                return {};

            Tensor input = concat_inputs(in.sensory_input, in.context_input);
            fit_to_size(input, input_size_);

            // Process through abstraction levels
            Tensor current = input;

            for (size_t level = 0; level < abstraction_levels_; ++level) {
                if (level > 0) {
                    fit_to_size(current, abstraction_nets_[level].input_size());
                }

                Tensor next = abstraction_nets_[level].predict(current);
                fit_to_size(next, output_size_);
                current = next;
                abstraction_cache_[level] = current;
            }

            return {current, current};
        }

        std::string name() const override { return name_; }
        std::size_t param_count() const override {
            size_t total = 0;
            for (const auto& net : abstraction_nets_) {
                total += param_count_from_layers({net.input_size(), net.output_size()});
            }
            return total;
        }

    private:
        std::string name_;
        std::size_t input_size_;
        std::size_t output_size_;
        std::size_t abstraction_levels_;
        std::vector<dnn::NeuralNetwork> abstraction_nets_;
        std::vector<Tensor> abstraction_cache_;
    };

    // Advanced Brain Simulation System with sophisticated learning algorithms
    class AdvancedBrainSimulation
    {
    public:
        AdvancedBrainSimulation(std::size_t sensory_size,
                               std::size_t action_count,
                               std::size_t context_size = 128)
            : sensory_size_(sensory_size),
              action_count_(action_count),
              context_size_(context_size),
              current_phase_(LearningPhase::ACQUISITION),
              phase_transition_threshold_(0.7),
              novelty_threshold_(0.3),
              memory_capacity_(4096),
              consolidation_frequency_(100)
        {
            // Initialize components
            engine_.set_context_size(context_size_);
            engine_.set_dt(0.02);
            engine_.set_settling_steps(3);
            engine_.set_context_blend(0.7);
            engine_.set_context_clip(4.0);

            // Create sophisticated multi-layered cognitive modules
            auto sensory_processor = std::make_unique<SensoryProcessingModule>(
                "sensory_processor", sensory_size_ + context_size_, context_size_);

            auto memory_consolidator = std::make_unique<MemoryConsolidationModule>(
                "memory_consolidator", sensory_size_ + context_size_, context_size_, context_size_ * 2, 0.005);
            memory_consolidator_ = memory_consolidator.get();

            auto pattern_recognizer = std::make_unique<PatternRecognitionModule>(
                "pattern_recognizer", context_size_ * 2, context_size_ / 2, 8);
            pattern_recognizer_ = pattern_recognizer.get();

            auto knowledge_abstractor = std::make_unique<KnowledgeAbstractionModule>(
                "knowledge_abstractor", context_size_ * 2, context_size_, 3);
            knowledge_abstractor_ = knowledge_abstractor.get();

            auto world_model = std::make_unique<WorldModelModule>(
                "world_model", sensory_size_, context_size_, context_size_ * 2);
            world_model_ = world_model.get();

            std::vector<std::size_t> policy_sizes{context_size_ * 2, context_size_, action_count_};
            auto policy = std::make_unique<PolicyModule>(
                "policy", policy_sizes, dnn::Activation::Relu, dnn::Activation::Linear);

            // Add modules to engine
            sensory_idx_ = engine_.add_module(std::move(sensory_processor), true, true);
            memory_idx_ = engine_.add_module(std::move(memory_consolidator), true, true);
            pattern_idx_ = engine_.add_module(std::move(pattern_recognizer), true, true);
            abstraction_idx_ = engine_.add_module(std::move(knowledge_abstractor), true, true);
            world_idx_ = engine_.add_module(std::move(world_model), true, true);
            policy_idx_ = engine_.add_module(std::move(policy), false, true);

            // Initialize random number generator
            rng_.seed(std::random_device{}());
        }

        // Process input and determine the learning phase
        [[nodiscard]] InputProcessingInfo process_input(const std::string& input_text)
        {
            InputProcessingInfo info;
            info.input_text = input_text;
            info.processed_tensor = encode_text(input_text);

            // Detect learning phase based on input characteristics
            info.current_phase = detect_learning_phase(input_text);

            // Extract concepts from input
            info.extracted_concepts = extract_concepts(input_text);

            // Calculate novelty score
            info.novelty_score = calculate_novelty(info.processed_tensor, info.extracted_concepts);

            // Check for conflicts with existing knowledge
            info.is_conflicting = detect_conflict(input_text, info.extracted_concepts);
            if (info.is_conflicting) {
                info.conflict_details = resolve_conflict(input_text, info.extracted_concepts);
            }

            // Update phase based on all factors
            update_learning_phase(info);

            // Store the processed information
            recent_inputs_.push_front(info);
            if (recent_inputs_.size() > 100) {
                recent_inputs_.pop_back();
            }

            return info;
        }

        // Main decision function with phase-aware processing
        [[nodiscard]] Decision make_decision(const std::string& input_text, double reward = 0.0)
        {
            auto processing_info = process_input(input_text);
            Tensor observation = processing_info.processed_tensor;

            // Adjust reward based on phase
            double adjusted_reward = reward;
            if (processing_info.current_phase == LearningPhase::ACQUISITION && reward > 0) {
                adjusted_reward *= 1.2; // Boost reward for learning phase
            } else if (processing_info.current_phase == LearningPhase::TESTING && reward > 0) {
                adjusted_reward *= 0.8; // Lower reward for testing phase
            }

            // Store the enhanced experience with phase information
            EnhancedExperience exp;
            exp.observation = observation;
            exp.context_before = engine_.context();
            exp.reward = adjusted_reward;
            exp.phase = processing_info.current_phase;
            exp.related_concepts = processing_info.extracted_concepts;

            // Perform action based on the current phase
            Tensor logits;
            switch (processing_info.current_phase) {
                case LearningPhase::ACQUISITION:
                    logits = acquisition_mode(observation, adjusted_reward);
                    break;
                case LearningPhase::CONSOLIDATION:
                    logits = consolidation_mode(observation, adjusted_reward);
                    break;
                case LearningPhase::RETRIEVAL:
                    logits = retrieval_mode(observation, adjusted_reward);
                    break;
                case LearningPhase::TESTING:
                    logits = testing_mode(observation, adjusted_reward);
                    break;
            }

            // Store the experience
            exp.context_after = engine_.context();
            exp.action_taken = logits;
            enhanced_experiences_.push_back(exp);
            if (enhanced_experiences_.size() > max_enhanced_experiences_) {
                enhanced_experiences_.erase(enhanced_experiences_.begin());
            }

            // Update knowledge hierarchy based on the outcome
            update_knowledge_hierarchy(processing_info, adjusted_reward);

            // Apply memory consolidation if needed
            if (step_count_ % consolidation_frequency_ == 0) {
                consolidate_memory();
            }

            // Calculate probabilities and make decision
            Tensor probs = softmax(logits, determine_temperature(processing_info.current_phase));
            int action = sample_from_probs(probs, rng_);

            Decision decision;
            decision.action = action;
            decision.logits = logits;
            decision.probs = probs;
            decision.value = value_estimate();

            step_count_++;

            return decision;
        }

        // Add knowledge to the hierarchy
        void add_knowledge(const std::string& concept_name, const std::vector<std::string>& related_concepts, double confidence = 1.0)
        {
            KnowledgeNode node;
            node.concept_name = concept_name;
            node.related_concepts = related_concepts;
            node.confidence = confidence;
            node.representation = encode_text(concept_name);

            knowledge_hierarchy_[concept_name] = node;

            // Update related concepts
            for (const auto& related : related_concepts) {
                auto it = knowledge_hierarchy_.find(related);
                if (it != knowledge_hierarchy_.end()) {
                    bool already_related = false;
                    for (const auto& existing : it->second.related_concepts) {
                        if (existing == concept_name) {
                            already_related = true;
                            break;
                        }
                    }
                    if (!already_related) {
                        it->second.related_concepts.push_back(concept_name);
                    }
                } else {
                    KnowledgeNode related_node;
                    related_node.concept_name = related;
                    related_node.related_concepts = {concept_name};
                    related_node.representation = encode_text(related);
                    knowledge_hierarchy_[related] = related_node;
                }
            }
        }

        // Enhanced conflict detection with more sophisticated reasoning
        bool has_conflict(const std::string& input_text) const
        {
            std::string lower_input = to_lower(input_text);

            // Check for direct contradictions (not X when X exists, is not X, etc.)
            for (const auto& [concept_key, node] : knowledge_hierarchy_) {
                if (lower_input.find("not " + concept_key) != std::string::npos ||
                    lower_input.find("is not " + concept_key) != std::string::npos ||
                    lower_input.find("no " + concept_key) != std::string::npos) {
                    return true;
                }

                // Check if the input contradicts a known relationship
                for (const auto& related : node.related_concepts) {
                    if (lower_input.find("not " + related) != std::string::npos ||
                        lower_input.find("is not " + related) != std::string::npos) {
                        return true;
                    }
                }
            }

            // Check for hierarchical contradictions (e.g., saying X is Y when X is known to be Z which conflicts with Y)
            std::vector<std::string> new_concepts = extract_concepts(input_text);
            for (const auto& new_concept : new_concepts) {
                auto existing_it = knowledge_hierarchy_.find(new_concept);
                if (existing_it != knowledge_hierarchy_.end()) {
                    // Check if the new statement contradicts existing relationships
                    if (lower_input.find("is") != std::string::npos) {
                        // Analyze "X is Y" statements for conflicts with existing "X is Z"
                        std::string::size_type pos = lower_input.find("is");
                        std::string subject = lower_input.substr(0, pos);
                        std::string predicate = lower_input.substr(pos + 2);

                        // Remove common words
                        std::vector<std::string> new_subject_concepts = extract_concepts(subject);
                        std::vector<std::string> new_predicate_concepts = extract_concepts(predicate);

                        for (const auto& s_concept : new_subject_concepts) {
                            auto s_it = knowledge_hierarchy_.find(s_concept);
                            if (s_it != knowledge_hierarchy_.end()) {
                                for (const auto& p_concept : new_predicate_concepts) {
                                    for (const auto& related : s_it->second.related_concepts) {
                                        if (p_concept == related) {
                                            // This might be consistent - skip conflict
                                            continue;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return false;
        }

        // Enhanced query with semantic similarity and fuzzy matching
        std::vector<std::string> query_knowledge(const std::string& query) const
        {
            std::vector<std::string> results;
            std::string lower_query = to_lower(query);

            // Direct match search
            for (const auto &[concept_key, node] : knowledge_hierarchy_)
            {
                if (lower_query.find(to_lower(concept_key)) != std::string::npos)
                {
                    results.push_back(node.concept_name + " (confidence: " + std::to_string(node.confidence) + ")");
                }
                // Also check if any related concepts match
                else {
                    for (const auto& related : node.related_concepts) {
                        if (lower_query.find(to_lower(related)) != std::string::npos) {
                            results.push_back(node.concept_name + " -> " + related + " (confidence: " + std::to_string(node.confidence) + ")");
                            break; // Don't add the same concept multiple times
                        }
                    }
                }
            }

            // If no direct matches, try semantic similarity
            if (results.empty()) {
                Tensor query_tensor = encode_text(query);

                for (const auto &[concept_key, node] : knowledge_hierarchy_)
                {
                    double similarity = calculate_similarity(query_tensor, node.representation);

                    if (similarity > 0.3) { // Threshold for semantic similarity
                        results.push_back(node.concept_name + " (similarity: " + std::to_string(similarity) + ", confidence: " + std::to_string(node.confidence) + ")");
                    }
                }

                // Sort by similarity if using semantic search
                std::sort(results.begin(), results.end(),
                         [](const std::string& a, const std::string& b) {
                             // Extract similarity values and compare
                             size_t pos_a = a.find("similarity: ");
                             size_t pos_b = b.find("similarity: ");
                             if (pos_a != std::string::npos && pos_b != std::string::npos) {
                                 pos_a += 12; // Length of "similarity: "
                                 pos_b += 12;

                                 std::string sim_str_a = a.substr(pos_a);
                                 std::string sim_str_b = b.substr(pos_b);

                                 // Extract the number part before next comma or parenthesis
                                 size_t end_a = sim_str_a.find(", ");
                                 size_t end_b = sim_str_b.find(", ");

                                 double sim_a = std::stod(sim_str_a.substr(0, end_a));
                                 double sim_b = std::stod(sim_str_b.substr(0, end_b));

                                 return sim_a > sim_b;
                             }
                             return a < b;
                         });
            }

            return results;
        }

        // Enhanced conflict resolution with hierarchical knowledge building
        std::string resolve_conflict(const std::string& input_text, const std::vector<std::string>& concepts)
        {
            std::string conflict_details = "Conflict detected and resolved";

            for (const auto &conceptvalue : concepts)
            {
                auto it = knowledge_hierarchy_.find(conceptvalue);
                if (it != knowledge_hierarchy_.end()) {
                    // Check if we're adding more specific information to an existing concept
                    std::string lower_input = to_lower(input_text);

                    // If the new input contains more specific details, enhance the existing knowledge
                    if (lower_input.find(" is ") != std::string::npos) {
                        std::string::size_type pos = lower_input.find(" is ");
                        std::string subject = lower_input.substr(0, pos);
                        std::string description = lower_input.substr(pos + 4);

                        if (to_lower(conceptvalue) == subject)
                        {
                            // We're adding more detail to an existing concept
                            conflict_details += ": Enhanced '" + conceptvalue + "' with new description";

                            // Update the concept representation to include new information
                            Tensor new_rep = encode_text(input_text);
                            // Blend the old and new representations
                            if (it->second.representation.size() == new_rep.size()) {
                                for (size_t i = 0; i < it->second.representation.size(); ++i) {
                                    // Weight the new information but preserve some of the old
                                    it->second.representation[i] = 0.3 * it->second.representation[i] + 0.7 * new_rep[i];
                                }
                            } else {
                                it->second.representation = new_rep;
                            }

                            // Update confidence
                            it->second.confidence = std::min(1.0, it->second.confidence + 0.1);

                            // Extract additional related concepts from the description
                            std::vector<std::string> new_related = extract_concepts(description);
                            for (const auto& new_rel : new_related) {
                                bool exists = false;
                                for (const auto& existing_rel : it->second.related_concepts) {
                                    if (to_lower(existing_rel) == to_lower(new_rel)) {
                                        exists = true;
                                        break;
                                    }
                                }
                                if (!exists) {
                                    it->second.related_concepts.push_back(new_rel);
                                }
                            }

                            it->second.last_accessed = std::chrono::steady_clock::now();
                            it->second.access_count++;
                            break;
                        }
                    }

                    conflict_details += ": " + conceptvalue + " vs. " + input_text;
                }
            }

            return conflict_details;
        }

        // Get current learning phase
        LearningPhase get_current_phase() const { return current_phase_; }

        void set_seed(std::uint64_t seed) { rng_.seed(seed); }

        // Real-time learning curve optimization metrics
        struct LearningCurveMetrics {
            double performance{0.0};
            double learning_rate{0.0};
            double retention{0.0};
            double forgetting_factor{0.0};
            double optimal_batch_size{8.0};
            double difficulty{1.0};
        };

        // Public methods for testing learning optimization functionality
        LearningCurveMetrics calculate_learning_metrics() const {
            return const_cast<AdvancedBrainSimulation*>(this)->calculate_learning_metrics_impl();
        }

        void optimize_learning_curve() {
            const_cast<AdvancedBrainSimulation*>(this)->optimize_learning_curve_impl();
        }

        void selective_forgetting() {
            const_cast<AdvancedBrainSimulation*>(this)->selective_forgetting_impl();
        }

        void reinforce_important_memories() {
            const_cast<AdvancedBrainSimulation*>(this)->reinforce_important_memories_impl();
        }

    private:
        std::size_t sensory_size_;
        std::size_t action_count_;
        std::size_t context_size_;
        BrainEngine engine_;
        int sensory_idx_{-1};
        int memory_idx_{-1};
        int pattern_idx_{-1};
        int abstraction_idx_{-1};
        int world_idx_{-1};
        int policy_idx_{-1};
        WorldModelModule* world_model_{nullptr};
        MemoryConsolidationModule* memory_consolidator_{nullptr};
        PatternRecognitionModule* pattern_recognizer_{nullptr};
        KnowledgeAbstractionModule* knowledge_abstractor_{nullptr};

        std::map<std::string, KnowledgeNode> knowledge_hierarchy_;
        std::vector<EnhancedExperience> enhanced_experiences_;
        std::size_t max_enhanced_experiences_{1024};

        LearningPhase current_phase_;
        double phase_transition_threshold_;
        double novelty_threshold_;
        std::size_t memory_capacity_;
        std::size_t consolidation_frequency_;
        std::deque<InputProcessingInfo> recent_inputs_;

        // Error correction and adaptive weight adjustment components
        std::vector<double> prediction_errors_;
        std::vector<double> adaptive_weights_;
        std::vector<double> error_history_;
        double error_correction_rate_{0.01};
        double weight_adaptation_rate_{0.005};
        std::size_t error_history_size_{100};

        std::mt19937_64 rng_;
        std::size_t step_count_{0};

        // Update error correction based on prediction accuracy
        void update_error_correction(const EnhancedExperience& exp)
        {
            // Calculate prediction error
            double predicted_reward = exp.expected_reward;
            double actual_reward = exp.reward;
            double error = std::abs(actual_reward - predicted_reward);

            prediction_errors_.push_back(error);
            if (prediction_errors_.size() > error_history_size_) {
                prediction_errors_.erase(prediction_errors_.begin());
            }

            // Adjust adaptive weights based on error patterns
            if (prediction_errors_.size() >= 2) {
                // Calculate error trend
                double recent_avg = 0.0;
                double prev_avg = 0.0;

                size_t mid_point = prediction_errors_.size() / 2;

                for (size_t i = 0; i < mid_point; ++i) {
                    prev_avg += prediction_errors_[i];
                }
                prev_avg /= static_cast<double>(mid_point);

                for (size_t i = mid_point; i < prediction_errors_.size(); ++i) {
                    recent_avg += prediction_errors_[i];
                }
                recent_avg /= static_cast<double>(prediction_errors_.size() - mid_point);

                // Adjust weights based on error trend
                if (recent_avg > prev_avg * 1.1) { // Error is increasing
                    // Increase adaptation rate to correct faster
                    error_correction_rate_ = std::min(error_correction_rate_ * 1.05, 0.1);
                    weight_adaptation_rate_ = std::min(weight_adaptation_rate_ * 1.02, 0.05);
                } else if (recent_avg < prev_avg * 0.9) { // Error is decreasing
                    // Decrease adaptation rate for stability
                    error_correction_rate_ = std::max(error_correction_rate_ * 0.95, 0.005);
                    weight_adaptation_rate_ = std::max(weight_adaptation_rate_ * 0.98, 0.001);
                }
            }

            // Update neural network weights based on error
            update_network_weights(exp);
        }

        // Update neural network weights adaptively
        void update_network_weights(const EnhancedExperience& exp)
        {
            // This is a simplified weight update - in a real implementation,
            // this would interface with the actual network parameters
            for (auto& weight : adaptive_weights_) {
                // Apply small adjustments based on experience importance
                weight += (exp.importance - 0.5) * weight_adaptation_rate_;
                // Keep weights in reasonable range
                weight = std::clamp(weight, -2.0, 2.0);
            }

            // If we have a world model, train it with the new experience
            if (world_model_ && step_count_ > 10) {  // Wait for some initial data
                // Create training data from experience
                std::vector<Tensor> X = {exp.observation};
                std::vector<Tensor> Y = {exp.next_obs};
                world_model_->train(X, Y, 1, 1, 0.01 * exp.importance);
            }
        }

        // Calculate prediction accuracy
        double calculate_prediction_accuracy() const
        {
            if (prediction_errors_.empty()) {
                return 1.0; // Assume perfect initially
            }

            double avg_error = 0.0;
            for (double error : prediction_errors_) {
                avg_error += error;
            }
            avg_error /= static_cast<double>(prediction_errors_.size());

            // Convert to accuracy (lower error = higher accuracy)
            return 1.0 / (1.0 + avg_error);
        }





        // Helper function to encode text to tensor
        Tensor encode_text(const std::string& text) const
        {
            Tensor v(sensory_size_, 0.0);
            const std::size_t len = std::min(sensory_size_, text.size());
            for (std::size_t i = 0; i < len; ++i) {
                const unsigned char c = static_cast<unsigned char>(text[i]);
                const double norm = (static_cast<double>(c) / 127.0) * 2.0 - 1.0;
                v[i] = norm;
            }
            return v;
        }

        // Helper function to convert string to lowercase
        std::string to_lower(std::string s) const
        {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });
            return s;
        }

        // Phase detection based on input characteristics
        LearningPhase detect_learning_phase(const std::string& input_text)
        {
            std::string lower_input = to_lower(input_text);

            // Check for question patterns
            bool is_question = input_text.find('?') != std::string::npos;
            if (is_question) {
                return LearningPhase::RETRIEVAL; // Questions often involve retrieval
            }

            // Check for learning patterns
            if (lower_input.find("what is") != std::string::npos ||
                lower_input.find("how does") != std::string::npos ||
                lower_input.find("explain") != std::string::npos ||
                lower_input.find("learn") != std::string::npos) {
                return LearningPhase::ACQUISITION;
            }

            // Check for testing patterns
            if (lower_input.find("test") != std::string::npos ||
                lower_input.find("quiz") != std::string::npos ||
                lower_input.find("evaluate") != std::string::npos) {
                return LearningPhase::TESTING;
            }

            // Default to acquisition if this is new content
            if (recent_inputs_.empty()) {
                return LearningPhase::ACQUISITION;
            }

            // Otherwise, return the current phase
            return current_phase_;
        }

        // Extract concepts from text
        std::vector<std::string> extract_concepts(const std::string& input_text) const
        {
            std::vector<std::string> concepts;
            std::string lower_input = to_lower(input_text);

            // Simple concept extraction - in a real system this would be more sophisticated
            std::vector<std::string> potential_concepts;

            // Split by common delimiters
            std::string word;
            for (char c : lower_input) {
                if (std::isalnum(c) || c == ' ') {
                    word += c;
                } else {
                    if (!word.empty()) {
                        if (word.size() > 2) { // Filter out short words
                            potential_concepts.push_back(word);
                        }
                        word.clear();
                    }
                }
            }
            if (!word.empty() && word.size() > 2) {
                potential_concepts.push_back(word);
            }

            // Filter and process concepts
            for (const auto &conceptvalue : potential_concepts)
            {
                concepts.push_back(conceptvalue);
            }

            return concepts;
        }

        // Calculate novelty of input
        [[nodiscard]] double calculate_novelty(const Tensor& tensor, const std::vector<std::string>& /*concepts*/)
        {
            if (knowledge_hierarchy_.empty()) {
                return 1.0; // Completely novel if no knowledge exists
            }

            double min_similarity = 1.0;

            // Find most similar existing knowledge
            for (const auto &[concept_key, node] : knowledge_hierarchy_)
            {
                double sim = calculate_similarity(tensor, node.representation);
                min_similarity = std::min(min_similarity, 1.0 - sim);
            }

            return min_similarity;
        }

        // Check for conflicts with existing knowledge
        bool detect_conflict(const std::string& input_text, const std::vector<std::string>& /*concepts*/)
        {
            // Currently concepts parameter is not used directly in the logic,
            // but we check for conflicts using has_conflict method
            if (has_conflict(input_text)) {
                return true;
            }
            return false;
        }

        // Update learning phase based on multiple factors
        void update_learning_phase(InputProcessingInfo& info)
        {
            // Adjust phase based on novelty
            if (info.novelty_score > 0.8) {
                info.current_phase = LearningPhase::ACQUISITION;
            } else if (info.novelty_score < 0.2) {
                info.current_phase = LearningPhase::RETRIEVAL; // Familiar content
            }

            // Adjust phase based on conflict status
            if (info.is_conflicting) {
                info.current_phase = LearningPhase::CONSOLIDATION; // Need to consolidate conflicting info
            }

            // Calculate phase confidence
            if (info.current_phase == current_phase_) {
                info.phase_confidence = std::min(1.0, info.phase_confidence + 0.1);
            } else {
                info.phase_confidence = std::max(0.1, info.phase_confidence - 0.1);
            }

            // Update global phase if confident
            if (info.phase_confidence > phase_transition_threshold_) {
                current_phase_ = info.current_phase;
            }
        }

        // Calculate similarity between tensors
        double calculate_similarity(const Tensor& a, const Tensor& b) const
        {
            const std::size_t n = std::min(a.size(), b.size());
            if (n == 0) return 0.0;

            double dot = 0.0;
            double na = 1e-9;
            double nb = 1e-9;
            for (std::size_t i = 0; i < n; ++i)
            {
                dot += a[i] * b[i];
                na += a[i] * a[i];
                nb += b[i] * b[i];
            }
            return std::abs(dot) / std::sqrt(na * nb);
        }

        // Acquisition mode: Learning new information
        Tensor acquisition_mode(const Tensor& observation, double reward)
        {
            // In acquisition mode, we focus on learning and exploration
            Tensor input = observation;
            fit_to_size(input, sensory_size_ + 1);
            input.back() = reward * 1.2; // Enhanced reward in acquisition mode

            return engine_.step(input, policy_idx_);
        }

        // Consolidation mode: Strengthening and organizing learned information
        Tensor consolidation_mode(const Tensor& observation, double reward)
        {
            // In consolidation mode, we reinforce existing knowledge
            Tensor input = observation;
            fit_to_size(input, sensory_size_ + 1);
            input.back() = reward * 0.9; // Slightly reduced reward

            // Trigger consolidation by updating context more slowly
            engine_.set_context_blend(0.4);

            Tensor result = engine_.step(input, policy_idx_);

            // Reset context blend
            engine_.set_context_blend(0.7);

            return result;
        }

        // Retrieval mode: Accessing stored information
        Tensor retrieval_mode(const Tensor& observation, double reward)
        {
            // In retrieval mode, we focus on accessing stored knowledge
            Tensor input = observation;
            fit_to_size(input, sensory_size_ + 1);
            input.back() = reward; // Normal reward

            return engine_.step(input, policy_idx_);
        }

        // Testing mode: Evaluating learned knowledge
        Tensor testing_mode(const Tensor& observation, double reward)
        {
            // In testing mode, we evaluate performance without learning adjustments
            Tensor input = observation;
            fit_to_size(input, sensory_size_ + 1);
            input.back() = reward * 0.8; // Reduced reward

            return engine_.step(input, policy_idx_);
        }

        // Determine appropriate temperature based on phase
        double determine_temperature(LearningPhase phase) const
        {
            switch (phase) {
                case LearningPhase::ACQUISITION:
                    return 1.2; // Higher temperature for exploration in learning
                case LearningPhase::CONSOLIDATION:
                    return 0.7; // Lower temperature for reinforcement
                case LearningPhase::RETRIEVAL:
                    return 0.5; // Low temperature for focused retrieval
                case LearningPhase::TESTING:
                    return 0.3; // Lowest temperature for accurate testing
            }
            return 1.0;
        }

        // Update knowledge hierarchy based on experience
        void update_knowledge_hierarchy(const InputProcessingInfo& info, double reward)
        {
            for (const auto &conceptvalue : info.extracted_concepts)
            {
                auto it = knowledge_hierarchy_.find(conceptvalue);
                if (it != knowledge_hierarchy_.end()) {
                    // Update existing concept
                    it->second.access_count++;
                    it->second.last_accessed = std::chrono::steady_clock::now();

                    // Adjust confidence based on reward
                    if (reward > 0) {
                        it->second.confidence = std::min(1.0, it->second.confidence + 0.05 * reward);
                    } else {
                        it->second.confidence = std::max(0.1, it->second.confidence + 0.02 * reward);
                    }
                } else {
                    // Add new concept
                    KnowledgeNode node;
                    node.concept_name = conceptvalue;
                    node.representation = encode_text(conceptvalue);
                    node.confidence = (reward > 0) ? 0.5 + 0.3 * reward : 0.3;
                    node.access_count = 1;
                    knowledge_hierarchy_[conceptvalue] = node;
                }
            }
        }

        // Consolidate memory by reinforcing important information
        void consolidate_memory()
        {
            // Reinforce frequently accessed and high-confidence knowledge
            for (auto &[concept_key, node] : knowledge_hierarchy_)
            {
                // Boost confidence slightly for frequently accessed concepts
                if (node.access_count > 5) {
                    node.confidence = std::min(1.0, node.confidence + 0.01);
                }

                // Reset access count for next consolidation period
                node.access_count = 0;
            }
        }

        // Value estimation from the current state
        double value_estimate() const
        {
            // Simple value estimation based on current context
            const auto& context = engine_.context();
            if (context.empty()) return 0.0;

            double sum = 0.0;
            for (double val : context) {
                sum += val;
            }
            return sum / static_cast<double>(context.size());
        }

    private:
        // Private helper methods with implementations
        LearningCurveMetrics calculate_learning_metrics_impl() const
        {
            LearningCurveMetrics metrics;

            // Calculate performance based on recent experiences
            double total_performance = 0.0;
            int valid_count = 0;

            for (const auto& exp : enhanced_experiences_) {
                if (exp.reward != 0.0) { // Only count experiences with rewards
                    total_performance += exp.reward;
                    valid_count++;
                }
            }

            metrics.performance = (valid_count > 0) ? total_performance / valid_count : 0.0;

            // Calculate learning rate based on improvement over time
            if (enhanced_experiences_.size() > 20) {
                double early_performance = 0.0;
                double late_performance = 0.0;
                int early_count = 0, late_count = 0;

                for (size_t i = 0; i < enhanced_experiences_.size(); ++i) {
                    if (i < enhanced_experiences_.size() / 2) {
                        if (enhanced_experiences_[i].reward != 0.0) {
                            early_performance += enhanced_experiences_[i].reward;
                            early_count++;
                        }
                    } else {
                        if (enhanced_experiences_[i].reward != 0.0) {
                            late_performance += enhanced_experiences_[i].reward;
                            late_count++;
                        }
                    }
                }

                double early_avg = (early_count > 0) ? early_performance / early_count : 0.0;
                double late_avg = (late_count > 0) ? late_performance / late_count : 0.0;

                metrics.learning_rate = (late_avg - early_avg) / (early_avg > 1e-6 ? early_avg : 1.0);
            }

            // Calculate retention based on knowledge hierarchy stability
            int stable_concepts = 0;
            int total_concepts = 0;

            for (const auto& [concept_key, node] : knowledge_hierarchy_) {
                total_concepts++;
                if (node.confidence > 0.6) {
                    stable_concepts++;
                }
            }

            metrics.retention = (total_concepts > 0) ? static_cast<double>(stable_concepts) / total_concepts : 1.0;

            // Estimate forgetting factor based on knowledge decay
            metrics.forgetting_factor = 1.0 - metrics.retention;

            // Estimate optimal batch size based on current performance
            if (metrics.performance > 0.7) {
                metrics.optimal_batch_size = 16.0; // Larger batches when performing well
            } else if (metrics.performance > 0.4) {
                metrics.optimal_batch_size = 8.0;  // Medium batches
            } else {
                metrics.optimal_batch_size = 4.0;  // Smaller batches for more frequent updates
            }

            // Estimate difficulty of current concepts
            if (prediction_errors_.size() > 10) {
                double error_variance = 0.0;
                double mean_error = 0.0;

                for (double err : prediction_errors_) {
                    mean_error += err;
                }
                mean_error /= prediction_errors_.size();

                for (double err : prediction_errors_) {
                    error_variance += (err - mean_error) * (err - mean_error);
                }
                error_variance /= prediction_errors_.size();

                metrics.difficulty = 1.0 + error_variance; // Higher variance = more difficult concepts
            }

            return metrics;
        }

        void optimize_learning_curve_impl()
        {
            LearningCurveMetrics metrics = calculate_learning_metrics_impl();

            // Adjust neural network learning rate based on performance
            double base_lr = 0.01;
            double adjusted_lr = base_lr;

            if (metrics.learning_rate < -0.1) {  // Performance is degrading
                adjusted_lr *= 0.5;  // Reduce learning rate to stabilize
            } else if (metrics.learning_rate > 0.2) {  // Rapid improvement
                adjusted_lr *= 1.5;  // Increase learning rate to capitalize
            }

            // The neural network doesn't have direct access to learning rate here,
            // but in a real implementation, we'd pass this to the training method

            // Adjust consolidation frequency based on retention
            if (metrics.retention < 0.5) {
                consolidation_frequency_ = 50;  // Consolidate more frequently
            } else if (metrics.retention > 0.8) {
                consolidation_frequency_ = 150; // Consolidate less frequently
            }

            // Adjust phase transition thresholds based on performance
            if (metrics.performance < 0.3) {
                // Be more exploratory when performing poorly
                phase_transition_threshold_ = 0.5;  // Lower threshold for phase changes
            } else if (metrics.performance > 0.8) {
                // Be more stable when performing well
                phase_transition_threshold_ = 0.8;  // Higher threshold for phase changes
            }
        }

        void selective_forgetting_impl()
        {
            auto now = std::chrono::steady_clock::now();

            // Prune knowledge nodes based on multiple factors
            for (auto it = knowledge_hierarchy_.begin(); it != knowledge_hierarchy_.end();) {
                const auto& node = it->second;

                // Calculate a forgetting score based on multiple factors
                double forgetting_score = 0.0;

                // Factor 1: Low confidence
                forgetting_score += (1.0 - node.confidence) * 0.4;

                // Factor 2: Low access frequency (how often it's been accessed relative to age)
                auto age = std::chrono::duration_cast<std::chrono::hours>(now - node.creation_time).count();
                if (age > 0) {
                    double access_rate = static_cast<double>(node.access_count) / (age + 1.0);
                    // Lower access rate increases forgetting score
                    forgetting_score += std::max(0.0, (1.0 - access_rate)) * 0.3;
                }

                // Factor 3: Recency (how long since last access)
                auto time_since_access = std::chrono::duration_cast<std::chrono::hours>(now - node.last_accessed).count();
                // More recent access = lower forgetting score
                forgetting_score += std::min(1.0, time_since_access / 168.0) * 0.3; // 168 hours = 1 week

                // Apply forgetting threshold
                if (forgetting_score > 0.7) {  // High forgetting score means forget this concept
                    it = knowledge_hierarchy_.erase(it);
                } else {
                    ++it;
                }
            }

            // Also apply forgetting to experiences
            if (enhanced_experiences_.size() > 512) {  // Only if we have enough experiences
                // Sort experiences by importance and keep only the most important ones
                std::sort(enhanced_experiences_.begin(), enhanced_experiences_.end(),
                         [](const EnhancedExperience& a, const EnhancedExperience& b) {
                             return a.importance > b.importance;
                         });

                // Keep only the top 50% of experiences by importance
                size_t retain_count = enhanced_experiences_.size() / 2;
                enhanced_experiences_.erase(enhanced_experiences_.begin() + retain_count, enhanced_experiences_.end());
            }
        }

        void reinforce_important_memories_impl()
        {
            // Increase confidence and access count for high-importance concepts
            for (auto& [concept_key, node] : knowledge_hierarchy_) {
                if (node.confidence < 0.3) {
                    // If confidence is very low, it might be due for reinforcement
                    // or could be a candidate for forgetting
                    if (node.access_count < 3) {
                        // Very low confidence and rarely accessed - reduce importance
                        node.confidence *= 0.9;
                    } else {
                        // Low confidence but accessed multiple times - reinforce
                        node.confidence = std::min(1.0, node.confidence + 0.05);
                    }
                } else if (node.confidence > 0.8) {
                    // High confidence concept - maintain or slightly increase
                    node.confidence = std::min(1.0, node.confidence + 0.01);
                }
            }
        }
    };

} // namespace brain
