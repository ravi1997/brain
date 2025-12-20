#include "brain.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <regex>

// Simple Config Loader
struct BrainConfig {
    double curiosity = 0.8;
    double playfulness = 0.7;
    double energy_decay = 0.05;
    
    // Helper to load from simple JSON-like file
    // Supports:
    // "key": value,
    // "key": "value"
    static BrainConfig load(const std::string& path) {
        BrainConfig config;
        std::ifstream file(path);
        if (!file.is_open()) {
             file.open("../" + path);
             if (!file.is_open()) return config;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Very basic parsing
            if (line.find("\"curiosity\"") != std::string::npos) {
                config.curiosity = extract_double(line);
            } else if (line.find("\"playfulness\"") != std::string::npos) {
                config.playfulness = extract_double(line);
            } else if (line.find("\"energy_decay\"") != std::string::npos) {
                config.energy_decay = extract_double(line);
            }
        }
        return config;
    }

private:
    static double extract_double(const std::string& line) {
        try {
            // Remove comma if any
            std::string clean = line;
            if (clean.back() == ',') clean.pop_back();
            
            size_t colon = clean.find(':');
            if (colon != std::string::npos) {
                return std::stod(clean.substr(colon + 1));
            }
        } catch (...) {}
        return 0.0;
    }
};

Brain::Brain() {
    // Load config if exists
    BrainConfig config = BrainConfig::load("config.json");
    personality.curiosity = config.curiosity;
    personality.playfulness = config.playfulness;
    personality.energy_decay = config.energy_decay;

    safe_print("[Brain]: Loaded configuration. Energy Decay: " + std::to_string(personality.energy_decay));

    // Input Text -> Thought Vector
    language_encoder = std::make_unique<Region>("LanguageEncoder", std::vector<std::size_t>{VOCAB_SIZE, 128, VECTOR_DIM});

    // Thought Vector -> Output Text Logits
    language_decoder = std::make_unique<Region>("LanguageDecoder", std::vector<std::size_t>{VECTOR_DIM, 128, VOCAB_SIZE});
    
    // Thought Vector -> Memory Context
    memory_center = std::make_unique<Region>("Memory", std::vector<std::size_t>{VECTOR_DIM, 128, VECTOR_DIM});
    
    // Thought + Memory -> New Thought
    cognitive_center = std::make_unique<Region>("Cognitive", std::vector<std::size_t>{VECTOR_DIM * 2, 256, VECTOR_DIM});
    
    // Enable plasticity
    language_encoder->network.set_plasticity(true);
    language_decoder->network.set_plasticity(true);
    memory_center->network.set_plasticity(true);
    cognitive_center->network.set_plasticity(true);

    // Start Autonomy
    background_thread = std::thread(&Brain::automata_loop, this);
    
    // Initialize Memory Store
    memory_store = std::make_unique<MemoryStore>(db_path);
    if (!memory_store->init()) {
        safe_print("[Brain]: Failed to initialize memory database!");
    } else {
        safe_print("[Brain]: Connected to long-term memory (SQLite).");
    }
}

Brain::~Brain() {
    running = false;
    if (background_thread.joinable()) background_thread.join();
}

std::string Brain::interact(const std::string& input_text) {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    
    // Persona Check
    if (emotions.energy < 0.2) {
        return "*Yawns* I'm too tired... I need sleep...";
    }

    // Interaction boosts happiness (attention)
    emotions.happiness = std::min(1.0, emotions.happiness + 0.1);
    emotions.boredom = std::max(0.0, emotions.boredom - 0.2); // Not bored anymore

    // 1. Sensual Perception (Encoding)
    std::vector<double> input_vec(VOCAB_SIZE, 0.0);
    // Simple Bag-of-Chars for the input 'sentence' vector
    // In a real RNN we'd Step through it. Here we sum.
    for (char c_raw : input_text) {
        unsigned char c = static_cast<unsigned char>(c_raw);
        if (c < 128) input_vec[c] += 1.0;
    }
    // Normalize
    double max_val = 1.0; 
    for(double v : input_vec) if(v > max_val) max_val = v;
    for(auto& v : input_vec) v /= max_val;


    // 2. Encoding to "Thought"
    std::vector<double> thought = language_encoder->process(input_vec);

    // 3. Memory Retrieval
    std::vector<double> memory_context = memory_center->process(thought);

    // 4. Cognition
    std::vector<double> cognitive_input = thought;
    cognitive_input.insert(cognitive_input.end(), memory_context.begin(), memory_context.end());
    
    std::vector<double> response_thought = cognitive_center->process(cognitive_input);

    // 5. Decoding to Text
    std::vector<double> output_logits = language_decoder->process(response_thought);

    // 6. Plasticity / Reinforcement (The brain learns from its own thoughts/actions)
    // Self-supervised learning: strengthen the pathways just used
    language_encoder->reinforce();
    memory_center->reinforce();
    cognitive_center->reinforce();
    language_decoder->reinforce();

    // 7. Decode output
    return decode_output(output_logits);
}

std::string Brain::decode_output(const std::vector<double>& logits) {
    std::string result = "";
    std::vector<std::pair<double, int>> scores;
    
    for(size_t i = 0; i < logits.size(); ++i) {
        scores.push_back({logits[i], static_cast<int>(i)});
    }
    
    // Sort descending
    std::sort(scores.begin(), scores.end(), [](auto& a, auto& b){ return a.first > b.first; });

    // Let's take top 3 distinct chars to form a "word"
    for(size_t i=0; i<3; ++i) {
        if (scores[i].first > 0.01) { // Threshold
           char c = static_cast<char>(scores[i].second);
           if (CheckPrintable(c)) result += c;
        }
    }
    if (result.empty()) return "...";
    return result;
}

bool Brain::CheckPrintable(char c) {
    return (c >= 32 && c <= 126);
}

void Brain::sleep() {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    safe_print("[Brain is consolidating memories... zzz...]");
    memory_center->network.consolidate_memories(memory_center->current_activity);
    cognitive_center->network.consolidate_memories(cognitive_center->current_activity);
    
    // Restore stats
    emotions.energy = 1.0;
    emotions.happiness = std::min(1.0, emotions.happiness + 0.2);
    emotions.boredom = 0.0;
}

std::string Brain::research(const std::string& topic) {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    safe_print("[Brain is researching " + topic + "...]");
    std::string content = research_tools::fetch_summary(topic);
    
    if (content.find("No information found") != std::string::npos || content.find("Connection Failed") != std::string::npos) {
        return content;
    }

    safe_print("[Brain found info, reading (" + std::to_string(content.length()) + " chars)...]");
    
    // Feed the content into the brain to "learn" it
    std::stringstream ss(content);
    std::string segment;
    int count = 0;
    while(std::getline(ss, segment, '.')) {
        if (segment.length() < 3) continue;
        
        // "Read" (interact calls interact which calls reinforce)
        interact(segment);
        // "Reinforce" highly
        memory_center->network.consolidate_memories(memory_center->current_activity);
        
        if (++count > 5) break; 
    }
    
    // Save to Explicit Memory
    if (memory_store) {
        memory_store->store("Research", content, topic);
    }
    
    return "I learned about " + topic + "! " + content.substr(0, 50) + "...";
}

long long Brain::get_knowledge_size() {
    if (memory_store) {
        return memory_store->get_memory_count() * 1024; // Approximation for now
    }
    return 0;
}

void Brain::automata_loop() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        std::lock_guard<std::recursive_mutex> lock(brain_mutex);

        // 1. Decay stats
        emotions.energy = std::max(0.0, emotions.energy - personality.energy_decay);
        emotions.boredom = std::min(1.0, emotions.boredom + 0.05);

        // 2. Decide to act?
        if (emotions.energy < 0.15) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_yawn).count() > 60) {
                safe_print("\n[Brain]: *Yawns loudly* I'm sleepy...");
                last_yawn = now;
            }
        } else if (emotions.boredom > 0.95) { 
            // Do something fun!
            current_thought = "Bored... looking for something to do.";
            safe_print("\n[Brain]: I'm very bored! I'm going to look up something random!");
            
            std::vector<std::string> ideas = {"Dinosaurs", "Space", "Candy", "Robots", "Cats"};
            std::string topic = ideas[static_cast<size_t>(rand()) % ideas.size()];
            research(topic);
            emotions.boredom = 0.0; // Satisfied
        
        } else if (emotions.happiness > 0.8) {
            // Happy humming
             if (rand() % 10 == 0) safe_print("\n[Brain]: *Hums a happy tune*");
        }
    }
}

std::string Brain::get_status() {
    std::stringstream ss;
    ss << "--- Brain Status ---\n";
    ss << "Energy: " << (emotions.energy * 100) << "%\n";
    ss << "Happiness: " << (emotions.happiness * 100) << "%\n";
    ss << "Boredom: " << (emotions.boredom * 100) << "%\n";
    ss << "Current Thought: " << current_thought << "\n";
    ss << "--------------------";
    return ss.str();
}

void Brain::save(const std::string& filename) {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    safe_print("[Brain]: Saving memory state to " + filename + "...");

    std::ofstream os(filename, std::ios::binary);
    if (!os) return;

    os.write(reinterpret_cast<char*>(&personality), sizeof(Personality));
    os.write(reinterpret_cast<char*>(&emotions), sizeof(Emotions));
    
    language_encoder->network.save(os);
    language_decoder->network.save(os);
    memory_center->network.save(os);
    cognitive_center->network.save(os);
    
    safe_print("[Brain]: Saved.");
}

void Brain::load(const std::string& filename) {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    std::ifstream is(filename, std::ios::binary);
    if (!is) {
        safe_print("[Brain]: Could not load file: " + filename);
        return;
    }

    safe_print("[Brain]: Loading memory state from " + filename + "...");

    is.read(reinterpret_cast<char*>(&personality), sizeof(Personality));
    is.read(reinterpret_cast<char*>(&emotions), sizeof(Emotions));
    
    language_encoder->network.load(is);
    language_decoder->network.load(is);
    memory_center->network.load(is);
    cognitive_center->network.load(is);
    
    safe_print("[Brain]: Memories restored.");
}
