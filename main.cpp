#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>

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

    // Enhanced function for parsing input and understanding context
    std::vector<std::string> extract_key_concepts(const std::string& input)
    {
        std::vector<std::string> concepts;
        std::string lower_input = to_lower(input);

        // Tokenize simple concepts
        std::istringstream iss(lower_input);
        std::string token;
        while (iss >> token) {
            // Remove punctuation and filter by length
            std::string clean_token;
            for (char c : token) {
                if (std::isalnum(c)) {
                    clean_token += c;
                }
            }
            if (clean_token.length() >= 3 && clean_token.length() <= 20) {
                concepts.push_back(clean_token);
            }
        }

        return concepts;
    }
}

int main()
{
    using namespace brain;
    constexpr std::size_t sensory_width = 128;  // Increased for better representation
    constexpr std::size_t action_count = 8;     // More complex actions

    // Create the advanced brain simulation
    AdvancedBrainSimulation advanced_brain(sensory_width, action_count, 256);
    advanced_brain.set_seed(42);

    std::cout << "+--------------------------------------------------------+\n";
    std::cout << "|      ADVANCED BRAIN SIMULATION SYSTEM v2.0              |\n";
    std::cout << "|  Mimicking Human Neural Learning Patterns                |\n";
    std::cout << "|  Adaptive Neural Networks with Dynamic Input Processing  |\n";
    std::cout << "+--------------------------------------------------------+\n";
    std::cout << "\n";
    std::cout << "System ready. Available commands:\n";
    std::cout << "  - Type statements to learn (e.g., 'mango is a fruit')\n";
    std::cout << "  - Type questions to retrieve knowledge (e.g., 'what is mango?')\n";
    std::cout << "  - Type 'test' to enter testing phase\n";
    std::cout << "  - Type 'status' to check current learning phase\n";
    std::cout << "  - Type 'knowledge' to see stored concepts\n";
    std::cout << "  - Type 'quit' to exit\n\n";

    std::string line;
    int step = 0;

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line == "quit" || line == "exit" || line == "stop")
        {
            std::cout << "Shutting down the brain simulation...\n";
            break;
        }

        if (line == "status")
        {
            std::cout << "Current learning phase: ";
            switch (advanced_brain.get_current_phase())
            {
                case LearningPhase::ACQUISITION:
                    std::cout << "ACQUISITION (Learning new information)\n";
                    break;
                case LearningPhase::CONSOLIDATION:
                    std::cout << "CONSOLIDATION (Strengthening memories)\n";
                    break;
                case LearningPhase::RETRIEVAL:
                    std::cout << "RETRIEVAL (Accessing stored knowledge)\n";
                    break;
                case LearningPhase::TESTING:
                    std::cout << "TESTING (Evaluating learned knowledge)\n";
                    break;
            }
            continue;
        }

        if (line == "knowledge")
        {
            std::cout << "Stored concepts in the knowledge hierarchy:\n";
            auto knowledge_list = advanced_brain.query_knowledge("");  // Get all knowledge
            if (knowledge_list.empty()) {
                std::cout << "  No concepts stored yet.\n";
            } else {
                for (const auto& concept_entry : knowledge_list) {
                    std::cout << "  - " << concept_entry << std::endl;
                }
            }
            continue;
        }

        // Process the input through the advanced brain simulation
        bool is_question = line.find('?') != std::string::npos;
        double reward = is_question ? 0.1 : 1.0;  // Lower reward for questions (retrieval), higher for learning
        if (line == "test") {
            reward = 0.5;  // Moderate reward for testing
        }

        Decision decision = advanced_brain.make_decision(line, reward);

        std::string response = "Processed: " + line;

        // Generate intelligent responses based on learning phase
        LearningPhase current_phase = advanced_brain.get_current_phase();
        switch (current_phase)
        {
            case LearningPhase::ACQUISITION:
                response = "Learning new concept: '" + line + "'";
                break;
            case LearningPhase::CONSOLIDATION:
                response = "Consolidating knowledge related to: '" + line + "'";
                break;
            case LearningPhase::RETRIEVAL:
                response = "Retrieved information: '" + line + "'";
                break;
            case LearningPhase::TESTING:
                response = "Testing knowledge application: '" + line + "'";
                break;
        }

        std::cout << "Response: " << response
                  << " | Phase: ";
        switch (current_phase)
        {
            case LearningPhase::ACQUISITION: std::cout << "Learn"; break;
            case LearningPhase::CONSOLIDATION: std::cout << "Consolidate"; break;
            case LearningPhase::RETRIEVAL: std::cout << "Retrieve"; break;
            case LearningPhase::TESTING: std::cout << "Test"; break;
        }
        std::cout << " | Action: " << decision.action
                  << " | Value: " << decision.value << std::endl;

        // Demonstrate hierarchical knowledge building
        if (step == 0) {
            std::cout << "\nExample: Adding initial knowledge to hierarchy...\n";
            advanced_brain.add_knowledge("fruit", {"edible", "sweet"}, 0.9);
            advanced_brain.add_knowledge("mango", {"fruit", "tropical"}, 0.8);
            advanced_brain.add_knowledge("apple", {"fruit", "red"}, 0.7);
        }

        // Demonstrate conflict resolution
        if (step == 5) {
            std::cout << "\nDemonstrating conflict resolution: Teaching 'mango is a sweet fruit' after 'mango is a fruit'...\n";
            advanced_brain.make_decision("mango is a sweet fruit", 1.0);
        }

        step++;
    }

    std::cout << "\nAdvanced Brain Simulation terminated.\n";
    std::cout << "Final learning phase: ";
    switch (advanced_brain.get_current_phase())
    {
        case LearningPhase::ACQUISITION: std::cout << "ACQUISITION\n"; break;
        case LearningPhase::CONSOLIDATION: std::cout << "CONSOLIDATION\n"; break;
        case LearningPhase::RETRIEVAL: std::cout << "RETRIEVAL\n"; break;
        case LearningPhase::TESTING: std::cout << "TESTING\n"; break;
    }
}
