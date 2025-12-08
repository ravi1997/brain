#include <iostream>
#include <random>

#include "brain.hpp"

int main()
{
    using namespace brain;
    CognitiveBrain brain(/*sensory*/ 8, /*actions*/ 4, /*context*/ 64);

    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    std::uniform_real_distribution<double> reward_dist(0.0, 1.0);
    brain::Tensor obs(8);
    for (double &v : obs)
    {
        v = dist(rng);
    }

    for (int step = 0; step < 20; ++step)
    {
        double reward = reward_dist(rng);
        Decision d = brain.decide(obs, reward, /*temperature*/ 0.8, /*greedy*/ false);

        std::cout << "step " << step << " action " << d.action
                  << " reward " << reward
                  << " value " << d.value
                  << " logits:";
        for (double l : d.logits)
        {
            std::cout << ' ' << l;
        }
        std::cout << " probs:";
        for (double p : d.probs)
        {
            std::cout << ' ' << p;
        }
        std::cout << '\n';

        brain::Tensor next_obs(8);
        for (double &v : next_obs)
        {
            v = dist(rng);
        }
        brain.record_transition(next_obs);
        obs = next_obs;

        if ((step + 1) % 5 == 0)
        {
            brain.learn_from_experience(/*epochs*/ 2, /*batch*/ 4, /*lr_value*/ 0.02, /*lr_world*/ 0.01);
        }
    }
}
