#pragma once

#include "dnn.hpp"
#include "memory_store.hpp"
#include "research_utils.hpp"
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <deque>
#include <atomic>
#include <mutex>
#include <map>
#include <filesystem>
#include <fstream>

// Simple thread-safe logger
inline void safe_print(const std::string& msg) {
    static std::mutex m;
    std::lock_guard<std::mutex> lock(m);
    std::cout << msg << std::endl;
}

struct Personality {
    double curiosity = 0.8;    // 0-1
    double playfulness = 0.7;  // 0-1
    double energy_decay = 0.05; 
};

struct Emotions {
    double happiness = 0.5; // 0-1
    double energy = 1.0;    // 0-1
    double boredom = 0.0;   // 0-1 (High = bored)
};

// Region represents a distinct functional area of the brain
class Region {
public:
    std::string name;
    dnn::NeuralNetwork network;
    std::vector<double> current_activity;

    Region(std::string n, const std::vector<std::size_t>& structure) 
        : name(std::move(n)), network(structure) {
        if (!structure.empty()) {
            current_activity.resize(structure.back(), 0.0);
        }
    }

    std::vector<double> process(const std::vector<double>& input) {
        // Cache input for learning
        last_input = input;
        current_activity = network.predict(input);
        return current_activity;
    }

    void train(const std::vector<double>& input, const std::vector<double>& target, double lr) {
        // dnn.hpp train takes vectors of vectors
        network.train({input}, {target}, 1, 1, lr);
    }

    // Hebbian Reinforcement: Train with Output as Target
    void reinforce(double intensity = 0.01) {
        if (!last_input.empty() && !current_activity.empty()) {
            train(last_input, current_activity, intensity); // Target = current_activity
        }
    }

private:
    std::vector<double> last_input;
};

class Brain {
public:
    std::unique_ptr<Region> language_encoder; // Broca's
    std::unique_ptr<Region> language_decoder; // Wernicke's
    std::unique_ptr<Region> memory_center;   // Hippocampus
    std::unique_ptr<Region> cognitive_center;// Prefrontal Cortex

    Personality personality;
    Emotions emotions;
    
    std::atomic<bool> running{true};
    mutable std::recursive_mutex brain_mutex; // Protects shared state (recursive to allow internal calls)
    std::thread background_thread;
    std::chrono::steady_clock::time_point last_yawn;
    std::string current_thought = "Idle";
    
    // Research Queue
    std::deque<std::string> research_queue;
    std::vector<std::string> learned_topics;

    // Explicit Memory (Database)
    std::unique_ptr<MemoryStore> memory_store;
    std::string db_path = "brain_memories.db";

    // Word-based hashing (1000 buckets)
    static constexpr size_t VOCAB_SIZE = 1000; 
    static constexpr size_t VECTOR_DIM = 64;  
    
    // Reverse mapping for decoding
    std::map<size_t, std::string> vocab_decode;

    Brain();
    ~Brain();

    std::string interact(const std::string& input_text);
    std::string decode_output(const std::vector<double>& logits);
    bool CheckPrintable(char c);
    
    // Helpers
    std::vector<std::string> tokenize(const std::string& text);
    void log_activity(const std::string& msg); // Background logging

    // "Sleep" / Consolidation
    void sleep();
    
    // Active Research
    std::string research(const std::string& topic);
    std::string deep_research(const std::string& topic);

    long long get_knowledge_size();

    // Autonomy Loop
    void automata_loop();

    std::string get_status();

    void save(const std::string& filename);
    void load(const std::string& filename);
};
