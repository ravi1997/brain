#include <algorithm>
#include <iostream>
#include <random>

#include "brain.hpp"

namespace
{
    int argmax(const brain::Tensor &v)
    {
        return static_cast<int>(std::distance(
            v.begin(), std::max_element(v.begin(), v.end())));
    }
}

int main()
{
    using namespace brain;
    BrainEngine engine;
    engine.set_context_size(32);
    engine.set_dt(0.05);

    auto vision = std::make_unique<VisionModule>(
        "vision", std::vector<std::size_t>{8, 32}, dnn::Activation::Relu, dnn::Activation::Tanh);
    auto memory = std::make_unique<MemoryModule>(
        "memory", /*sensory*/ 32, /*context*/ 32, /*hidden*/ 32, /*leak*/ 0.01);
    auto policy = std::make_unique<PolicyModule>(
        "policy", std::vector<std::size_t>{32 + 32, 32, 4}); // logits for 4 actions

    engine.add_module(std::move(vision), true, false);
    engine.add_module(std::move(memory), true, true);
    int policy_idx = engine.add_module(std::move(policy), false, true);

    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    brain::Tensor obs(8);

    for (int step = 0; step < 5; ++step)
    {
        for (double &v : obs)
        {
            v = dist(rng);
        }

        auto logits = engine.step(obs, policy_idx);
        int action = argmax(logits);

        std::cout << "step " << step
                  << " action " << action
                  << " logits:";
        for (double l : logits)
        {
            std::cout << ' ' << l;
        }
        std::cout << '\n';
    }
}
