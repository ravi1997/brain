#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "brain.hpp"

namespace
{
    brain::Tensor encode_text(const std::string &text, std::size_t width)
    {
        brain::Tensor v(width, 0.0);
        const std::size_t len = std::min(width, text.size());
        for (std::size_t i = 0; i < len; ++i)
        {
            const unsigned char c = static_cast<unsigned char>(text[i]);
            const double norm = (static_cast<double>(c) / 127.0) * 2.0 - 1.0;
            v[i] = norm;
        }
        return v;
    }

    double similarity(const brain::Tensor &a, const brain::Tensor &b)
    {
        const std::size_t n = std::min(a.size(), b.size());
        double dot = 0.0;
        double na = 1e-9;
        double nb = 1e-9;
        for (std::size_t i = 0; i < n; ++i)
        {
            dot += a[i] * b[i];
            na += a[i] * a[i];
            nb += b[i] * b[i];
        }
        return dot / std::sqrt(na * nb);
    }
}

int main()
{
    using namespace brain;
    constexpr std::size_t sensory_width = 64;
    CognitiveBrain brain(/*sensory*/ sensory_width, /*actions*/ 4, /*context*/ 128);
    brain.set_experience_limit(1024);
    brain.set_seed(42);

    struct MemoryEntry
    {
        std::string text;
        brain::Tensor embed;
    };
    std::vector<MemoryEntry> memory;

    std::cout << "Brain ready. Type to teach or ask (end with '?' to ask). Type 'stop' to exit.\n";
    std::string line;
    int step = 0;
    while (std::getline(std::cin, line))
    {
        if (line == "stop" || line == "quit" || line == "exit")
        {
            break;
        }

        const bool is_question = line.find('?') != std::string::npos;
        Tensor obs = encode_text(line, sensory_width);
        double reward = is_question ? 0.0 : 1.0;

        Decision d = brain.decide(obs, reward, /*temperature*/ is_question ? 0.6 : 1.0, /*greedy*/ is_question);

        std::string answer;
        if (is_question && !memory.empty())
        {
            double best = -1.0;
            std::size_t best_idx = 0;
            Tensor query = encode_text(line, sensory_width);
            for (std::size_t i = 0; i < memory.size(); ++i)
            {
                double s = similarity(query, memory[i].embed);
                if (s > best)
                {
                    best = s;
                    best_idx = i;
                }
            }
            if (best > 0.1)
            {
                answer = memory[best_idx].text;
            }
        }
        if (answer.empty() && is_question)
        {
            answer = "[thinking] action=" + std::to_string(d.action);
        }

        if (!is_question)
        {
            memory.push_back(MemoryEntry{line, obs});
        }

        brain.record_transition(obs);

        if ((step + 1) % 4 == 0)
        {
            brain.learn_from_experience(/*epochs*/ 1, /*batch*/ 8, /*lr_value*/ 0.01, /*lr_world*/ 0.005);
        }

        if (!is_question)
            answer = line;

        std::cout << (is_question ? "Answer: " : "Learned: ") << answer
                  << " | value " << d.value << " | action " << d.action << '\n';
        ++step;
    }
}
