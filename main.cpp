#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <chrono>

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

    std::string to_lower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                       { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    std::vector<std::size_t> topk_matches(const brain::Tensor &query,
                                          const std::vector<brain::Tensor> &embeds,
                                          std::size_t k,
                                          double min_sim)
    {
        std::vector<std::pair<double, std::size_t>> scored;
        scored.reserve(embeds.size());
        for (std::size_t i = 0; i < embeds.size(); ++i)
        {
            double s = similarity(query, embeds[i]);
            if (s >= min_sim)
            {
                scored.emplace_back(s, i);
            }
        }
        std::sort(scored.begin(), scored.end(),
                  [](auto &a, auto &b)
                  { return a.first > b.first; });
        std::vector<std::size_t> idx;
        for (std::size_t i = 0; i < std::min(k, scored.size()); ++i)
        {
            idx.push_back(scored[i].second);
        }
        return idx;
    }

    std::string synthesize_answer(const std::string &question,
                                  const std::vector<std::string> &memory_texts,
                                  const std::vector<std::size_t> &match_idx)
    {
        if (match_idx.empty())
            return {};

        std::string lower_q = to_lower(question);
        std::string subject = "It";
        auto pos = lower_q.find("what is ");
        if (pos != std::string::npos)
        {
            std::string tail = question.substr(pos + 7);
            if (!tail.empty())
            {
                subject = tail;
            }
        }
        else
        {
            pos = lower_q.find("what do you know about ");
            if (pos != std::string::npos)
            {
                std::string tail = question.substr(pos + 23);
                if (!tail.empty())
                {
                    subject = tail;
                }
            }
        }

        std::string answer = subject + " can be described as: ";
        for (std::size_t i = 0; i < match_idx.size(); ++i)
        {
            answer += memory_texts[match_idx[i]];
            if (i + 1 < match_idx.size())
            {
                answer += "; ";
            }
        }
        return answer;
    }

    std::vector<std::string> unique_phrases(const std::vector<std::string> &texts)
    {
        std::vector<std::string> out;
        for (const auto &t : texts)
        {
            const std::string lowered = to_lower(t);
            bool seen = false;
            for (const auto &o : out)
            {
                if (to_lower(o) == lowered)
                {
                    seen = true;
                    break;
                }
            }
            if (!seen)
                out.push_back(t);
        }
        return out;
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
        std::chrono::steady_clock::time_point t;
    };
    std::vector<MemoryEntry> memory;
    std::vector<brain::Tensor> memory_embeds;
    std::vector<std::string> memory_texts;

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
            Tensor query = encode_text(line, sensory_width);
            auto idx = topk_matches(query, memory_embeds, 3, 0.05);
            answer = synthesize_answer(line, memory_texts, idx);
        }
        if (answer.empty() && is_question)
        {
            answer = "[thinking] action=" + std::to_string(d.action);
        }

        if (!is_question)
        {
            memory.push_back(MemoryEntry{line, obs, std::chrono::steady_clock::now()});
            memory_embeds.push_back(obs);
            memory_texts.push_back(line);
        }

        brain.record_transition(obs);

        if ((step + 1) % 4 == 0)
        {
            brain.learn_from_experience(/*epochs*/ 1, /*batch*/ 8, /*lr_value*/ 0.01, /*lr_world*/ 0.005);
        }

        if (!is_question)
            answer = line;

        const auto now = std::chrono::steady_clock::now();
        if ((step % 6 == 0) && !memory.empty())
        {
            // Simple rehearsal: resurface older memories
            for (std::size_t i = 0; i < memory.size(); ++i)
            {
                auto age = std::chrono::duration_cast<std::chrono::seconds>(now - memory[i].t).count();
                if (age > 30)
                {
                    memory[i].t = now;
                    memory_embeds[i] = memory[i].embed;
                }
            }
        }

        if (memory.size() > 2048)
        {
            memory.erase(memory.begin(), memory.begin() + 512);
            memory_embeds.erase(memory_embeds.begin(), memory_embeds.begin() + 512);
            if (memory_texts.size() > 512)
                memory_texts.erase(memory_texts.begin(), memory_texts.begin() + 512);
        }

        if (is_question && answer.empty() && !memory_texts.empty())
        {
            auto unique = unique_phrases(memory_texts);
            if (!unique.empty())
            {
                answer = synthesize_answer(line, unique, std::vector<std::size_t>{0});
            }
        }

        std::cout << (is_question ? "Answer: " : "Learned: ") << answer
                  << " | value " << d.value << " | action " << d.action << '\n';
        ++step;
    }
}
