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
    CognitiveBrain brain(/*sensory*/ 8, /*actions*/ 4, /*context*/ 64);

    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    std::uniform_real_distribution<double> reward_dist(0.0, 1.0);
    brain::Tensor obs(8);

    for (int step = 0; step < 5; ++step)
    {
        for (double &v : obs)
        {
            v = dist(rng);
        }

        double reward = reward_dist(rng);
        auto logits = brain.act(obs, reward);
        int action = argmax(logits);

        std::cout << "step " << step
                  << " action " << action
                  << " reward " << reward
                  << " value " << brain.value_estimate()
                  << " logits:";
        for (double l : logits)
        {
            std::cout << ' ' << l;
        }
        std::cout << '\n';
    }
}
