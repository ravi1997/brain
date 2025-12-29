#pragma once

#include "dnn.hpp"
#include "memory_store.hpp"
#include "research_utils.hpp"
#include "reflex.hpp"
#include "sensory_unit.hpp"
#include <map>
#include "task_manager.hpp"
#include "redis_client.hpp"
#include "planning_unit.hpp"
#include "metacognition.hpp"
#include "tool_registry.hpp"
#include "planning_unit.hpp"
#include "metacognition.hpp"
#include "tool_registry.hpp"
#include "swarm_protocol.hpp"
#include "federation.hpp"
#include "hal.hpp"
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
#include <unordered_set>
#include "include/cognitive_core.hpp"


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

struct Emotions {
    double happiness = 0.5; // 0-1
    double sadness = 0.0;   // 0-1
    double anger = 0.0;     // 0-1
    double fear = 0.0;      // 0-1
    double energy = 1.0;    // 0-1
    double boredom = 0.0;   // 0-1 (High = bored)
};

// Feature 2: Theory of Mind
struct UserModel {
    double estimated_happiness = 0.5;
    double trust = 0.5;
    std::deque<std::string> intent_history;
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
    
    // NLU Helpers
    std::vector<std::string> extract_entities(const std::string& text);
    std::string resolve_intent(const std::string& text);
    void update_context(const std::string& role, const std::string& text, const std::string& intent);
    
    struct ContextItem {
        std::string role; // "User" or "Brain"
        std::string text;
        std::string intent;
        std::time_t timestamp;
        bool consolidated = false;
    };
    std::deque<ContextItem> conversation_history; // Stores last N turns

    Personality personality;
    Emotions emotions;
    Reflex reflex;
    std::string last_reflex_trigger;
    std::string last_reflex_response;
    void update_reflex_learning(const std::string& trigger, double reward);
    void consolidate_memories(); // Sleep cycle task
    std::vector<std::string> find_similar_concepts(const std::string& term);

    TaskManager task_manager;
    std::unique_ptr<PlanningUnit> planning_unit;
    
    std::atomic<bool> running{true};
    mutable std::recursive_mutex brain_mutex; // Protects shared state (recursive to allow internal calls)
    std::thread background_thread;
    std::chrono::steady_clock::time_point last_yawn;
    std::string current_thought = "Idle";
    
    // Research Queue
    std::deque<std::string> research_queue;
    std::vector<std::string> learned_topics;
    std::string current_research_topic = "None";

    // Sentiment Analysis
    double analyze_sentiment(const std::string& text);
    std::vector<std::string> positive_words;
    std::vector<std::string> negative_words;

    // Focus Mechanism
    double focus_level = 0.0; // 0..1
    std::string focus_topic = "None";

    // Explicit Memory (Database & Cache)
    std::unique_ptr<MemoryStore> memory_store;
    std::unique_ptr<RedisClient> redis_cache;
    std::string db_conn_str = "host=postgres dbname=brain_db user=brain_user password=brain_password";

    // Multi-Modal Sensory Bridge [Pillar 3]
    std::vector<std::unique_ptr<dnn::SensoryUnit>> sensory_inputs;
    void register_sensory_unit(std::unique_ptr<dnn::SensoryUnit> unit);
    void update_sensory_focus(); // Adjusts focus levels based on intent/context
    std::vector<double> get_aggregate_sensory_input();
    
    // Neural Bypass UI [Pillar 4]
    std::atomic<bool> bypass_enabled{true};
    void emit_neural_event(const std::string& type, const std::string& data);

    // Context Window (Short-term Conversation History)
    std::deque<std::string> conversation_context;
    static constexpr size_t MAX_CONTEXT_TURNS = 6; // Stores User + Brain pairs (3 turns)

    // Word-based hashing (10000 buckets)
    static constexpr size_t VOCAB_SIZE = 10000; 
    static constexpr size_t VECTOR_DIM = 384;  // Updated for pgvector compatibility (MiniLM standard)
    
    // Reverse mapping for decoding
    // Reverse mapping for decoding
    std::map<size_t, std::string> vocab_decode;
    
    // Synonym Mapping (Word -> Root Meaning)
    std::map<std::string, std::string> synonyms;
    std::map<std::string, std::vector<double>> word_embeddings;
    
    // NLU Helpers
    std::unordered_set<std::string> stopwords_;
    void load_stopwords();
    bool is_stop_word(const std::string& word);
    std::chrono::system_clock::time_point last_interaction_time;
    
    // Mega-Batch 11 New Components
    std::unique_ptr<dnn::Metacognition> metacognition;
    std::unique_ptr<dnn::ToolRegistry> tools;
    UserModel user_model; // Single user for now
    std::deque<dnn::SwarmPacket> swarm_queue;
    
    // Conditioned Reflexes: Stimulus Token -> Reflex Token
    std::map<std::string, std::string> condition_map;
    
    // Mega-Batch 12
    std::unique_ptr<dnn::FederationUnit> federation;
    std::unique_ptr<dnn::HardwareAccelerator> hardware;
    
    // Feature 5: Metabolism
    void metabolize_step();
    // Feature 6: REM Logic
    void perform_rem_cycle();

    // Cognitive Core - Unified access to all 100 AI features
    std::unique_ptr<dnn::CognitiveCore> cognitive_core;


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
    std::string get_memory_graph();
    std::string find_curiosity_topic(); // [Mega-Batch 5]

    long long get_knowledge_size();

    // Autonomy Loop
    void automata_loop();
    // Mega-Batch 6: Enhanced Planning
    void evaluate_goals();
    
    std::string get_status();
    std::string get_json_state();
    void update_from_json(const std::string& json);

    void save(const std::string& filename);
    void load(const std::string& filename);
    
    // ========== COGNITIVE CORE ACCESS METHODS ==========
    // Brain 2.0: Unified AI capabilities through cognitive_core
    
    /**
     * Deep reasoning about a query using causal inference, abduction, and explanation
     */
    std::string deep_reason(const std::string& query, const std::vector<std::string>& context = {});
    
    /**
     * Understand cause and effect relationships
     */
    float analyze_causality(const std::string& cause, const std::string& effect);
    
    /**
     * What-if counterfactual reasoning
     */
    std::string what_if(const std::string& variable, float new_value, const std::string& target);
    
    /**
     * Query common sense knowledge
     */
    std::vector<std::string> query_commonsense(const std::string& subject, const std::string& relation = "");
    
    /**
     * Meta-learn from few examples (adapt quickly)
     */
    void adapt_from_examples(const std::vector<std::pair<std::vector<float>, std::vector<float>>>& examples);
    
    /**
     * Get cognitive system status
     */
    std::string get_cognitive_status();
    
    // Supervised Learning

    void teach(const std::string& input, const std::string& target);
    
    // Event Callbacks
    std::function<void(const std::string&)> on_log;
    std::function<void(const std::string&)> on_error;
    std::function<void(const std::string&)> on_thought;
    std::function<void(const std::string&)> on_emotion_update;
    std::function<void(const std::string&, const std::string&)> on_neural_event;
    std::function<void(const std::string&)> on_research_update;
    
    void set_log_callback(std::function<void(const std::string&)> cb) { on_log = cb; }
    void set_error_callback(std::function<void(const std::string&)> cb) { on_error = cb; }
    void set_thought_callback(std::function<void(const std::string&)> cb) { on_thought = cb; }

private:
    void emit_log(const std::string& msg) { if(on_log) on_log(msg); else std::cout << msg << std::endl; }
    void emit_thought(const std::string& msg) { if(on_thought) on_thought(msg); }
};
