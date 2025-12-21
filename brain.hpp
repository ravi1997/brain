#pragma once

#include "dnn.hpp"
#include "memory_store.hpp"
#include "research_utils.hpp"
#include "reflex.hpp"
#include "task_manager.hpp"
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
    double friendliness = 0.5; // 0=Rude, 1=Kind
    double formality = 0.5;    // 0=Slang, 1=Polite
    double positivity = 0.5;   // 0=Sad/Depressed, 1=Happy/Cheery
    double energy_decay = 0.05; 
};

#include "cognition/emotion_unit.hpp"
#include "cognition/reasoning_unit.hpp"
#include "cognition/decision_unit.hpp"
#include "cognition/planning_unit.hpp"
#include "cognition/learning_unit.hpp"

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
    // Legacy Neural Regions (to be slowly phased out or integrated)
    std::unique_ptr<Region> language_encoder; 
    std::unique_ptr<Region> language_decoder; 
    std::unique_ptr<Region> memory_center;   
    std::unique_ptr<Region> cognitive_center;

    // Cognitive Units 2.0
    std::unique_ptr<EmotionUnit> emotion_unit;
    std::unique_ptr<ReasoningUnit> reasoning_unit;
    std::unique_ptr<DecisionUnit> decision_unit;
    std::unique_ptr<PlanningUnit> planning_unit;
    std::unique_ptr<LearningUnit> learning_unit;

    Personality personality;
    // Emotions emotions; // DEPRECATED: Replaced by emotion_unit
    Reflex reflex;
    TaskManager task_manager;
    
    std::atomic<bool> running{true};
    mutable std::recursive_mutex brain_mutex; 
    std::thread background_thread;
    std::chrono::steady_clock::time_point last_yawn;
    std::string current_thought = "Idle";
    
    // Research Queue
    std::deque<std::string> research_queue;
    std::vector<std::string> learned_topics;
    std::string current_research_topic = "None";

    // Explicit Memory (Database)
    std::unique_ptr<MemoryStore> memory_store;
    std::string db_path = "brain_memories.db";

    // Word-based hashing (10000 buckets)
    static constexpr size_t VOCAB_SIZE = 10000; 
    static constexpr size_t VECTOR_DIM = 256;  // Increased dimension for better capacity
    
    // Reverse mapping for decoding
    // Reverse mapping for decoding
    std::map<size_t, std::string> vocab_decode;
    
    // Synonym Mapping (Word -> Root Meaning)
    std::map<std::string, std::string> synonyms;

    Brain();
    ~Brain();

    std::string interact(const std::string& input_text);
    std::string decode_output(const std::vector<double>& logits);
    std::string get_associative_memory(const std::string& input);
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
    std::string get_json_state();
    void update_from_json(const std::string& json);

    void save(const std::string& filename);
    void load(const std::string& filename);
    
    // Supervised Learning
    void teach(const std::string& input, const std::string& target);
    
    // Event Callbacks
    std::function<void(const std::string&)> on_log;
    std::function<void(const std::string&)> on_error;
    std::function<void(const std::string&)> on_thought;
    std::function<void(const std::string&)> on_emotion_update;
    std::function<void(const std::string&)> on_research_update;
    
    void set_log_callback(std::function<void(const std::string&)> cb) { on_log = cb; }
    void set_error_callback(std::function<void(const std::string&)> cb) { on_error = cb; }
    void set_thought_callback(std::function<void(const std::string&)> cb) { on_thought = cb; }

private:
    void emit_log(const std::string& msg) { if(on_log) on_log(msg); else std::cout << msg << std::endl; }
    void emit_thought(const std::string& msg) { if(on_thought) on_thought(msg); }
};
