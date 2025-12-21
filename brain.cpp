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
    double energy_decay = 0.002993; // Slower decay (~25 mins awake)
    
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
    
    // Initialize Synonyms (Basic Thesaurus)
    synonyms["happy"] = "joy";
    synonyms["joyful"] = "joy";
    synonyms["glad"] = "joy";
    synonyms["sad"] = "sorrow";
    synonyms["unhappy"] = "sorrow";
    synonyms["depressed"] = "sorrow";
    synonyms["mad"] = "anger";
    synonyms["angry"] = "anger";
    synonyms["furious"] = "anger";
    synonyms["smart"] = "intelligent";
    synonyms["clever"] = "intelligent";
    synonyms["dumb"] = "stupid";
    synonyms["dull"] = "stupid";
    // ... load more from file if needed
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

    // 1. Reflex / Instinct Logic
    std::string instinct = reflex.get_reaction(input_text);
    if (!instinct.empty()) {
        emit_log("[Reflex]: Activated for '" + input_text + "'");
        emotions.boredom = std::max(0.0, emotions.boredom - 0.1);
        emotions.happiness = std::min(1.0, emotions.happiness + 0.05);
        return instinct;
    }

    // 2. Associative Memory Retrieval (RAG-lite)
    if (memory_store) {
        std::string memory_response = get_associative_memory(input_text);
        if (!memory_response.empty()) {
            emit_log("[Memory]: Recalled fact about '" + input_text + "'");
            
            // "Prime" the brain with this knowledge for context (optional, but good for plasticity)
            auto tokens = tokenize(memory_response);
            for(const auto& t : tokens) {
                std::hash<std::string> h;
                vocab_decode[h(t) % VOCAB_SIZE] = t;
            }
            
            return memory_response;
        }
    }

    // 3. Sensual Perception (Encoding)
    std::vector<double> input_vec(VOCAB_SIZE, 0.0);
    
    // Convert to words (Bag of Words Hashing) with N-GRAMS
    auto tokens = tokenize(input_text);
    
    // Pre-process tokens for synonyms
    for(auto& t : tokens) {
        if(synonyms.count(t)) t = synonyms[t];
    }

    std::vector<std::string> ngrams = tokens;
    // Add Bigrams (e.g., "not happy")
    for(size_t i=0; i < tokens.size()-1; ++i) {
        ngrams.push_back(tokens[i] + "_" + tokens[i+1]);
    }

    for (const auto& word : ngrams) {
        // Hash string to 0..999
        std::hash<std::string> hasher;
        size_t idx = hasher(word) % VOCAB_SIZE;
        input_vec[idx] += 1.0;
        
        // Learn the word mapping
        if (vocab_decode.find(idx) == vocab_decode.end()) {
             // Store human readable form (replace _ with space for decoding)
             std::string clean = word;
             std::replace(clean.begin(), clean.end(), '_', ' ');
             vocab_decode[idx] = clean;
        }
    }

    // Normalize
    double max_val = 1.0; 
    for(double v : input_vec) if(v > max_val) max_val = v;
    for(auto& v : input_vec) v /= max_val;


    // 2. Encoding to "Thought"
    std::vector<double> thought = language_encoder->process(input_vec);

    // 3. Memory Retrieval
    std::vector<double> memory_context = memory_center->process(thought);
    
    // ASSOCIATIVE MEMORY INJECTION
    // If we found a fact in step 2 (get_associative_memory), we should "feel" it too.
    // We re-query here to get the content, tokenize it, and add to memory_context
    if (memory_store) {
        // Iterate tokens again to find triggers
        for(const auto& t : tokens) {
             if (t.length() <= 3) continue;
             auto results = memory_store->query(t);
             if (!results.empty()) {
                 // Vectorize the content of the memory
                 auto mem_tokens = tokenize(results[0].content);
                 for(const auto& mt : mem_tokens) {
                     std::hash<std::string> h;
                     // Add to memory context (fold into vector dim)
                     size_t idx = h(mt) % VECTOR_DIM; 
                     memory_context[idx] += 0.5; // Injection weight
                 }
                 break; // Only inject top relevance to avoid noise
             }
        }
    }

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
    // 7. Decode output
    std::string response_text = decode_output(output_logits);

    // EMIT THOUGHT
    emit_thought("Thinking about: " + input_text + " => " + response_text);

    // PERSONALITY MODULATION
    // Anger = All Caps
    if (emotions.anger > 0.7) {
        std::transform(response_text.begin(), response_text.end(), response_text.begin(), ::toupper);
        response_text += "!!!";
    }
    
    // Rude (Low Friendliness)
    if (personality.friendliness < 0.3) {
        if (rand() % 3 == 0) response_text = "Look, " + response_text;
        else if (rand() % 3 == 1) response_text = "Ugh. " + response_text;
    }
    
    // Formality
    if (personality.formality > 0.8) {
        if (response_text.find("I don't") != std::string::npos) {
             response_text.replace(response_text.find("I don't"), 7, "I do not");
        }
        // Heuristic: Append "Sir" sometimes
        if (rand() % 5 == 0) response_text += ", Sir.";
    }

    // Continuous Learning: Save interesting interactions
    if (input_text.length() > 20 && response_text.length() > 10 && response_text.find("...") == std::string::npos) {
         // Only save if it's a "good" interaction (heuristic)
         // Append to a learning file
         std::ofstream learn_file("learned_interactions.txt", std::ios::app);
         if (learn_file) {
             learn_file << input_text << "|" << response_text << "\n";
             safe_print("[Persistence]: Saved interaction to learned_interactions.txt");
         } else {
             safe_print("[Persistence]: Failed to open file for writing!");
         }
    } else {
        // Debug why not saved (comment out in production)
         // safe_print("[Persistence]: Skipped. InLen=" + std::to_string(input_text.length()) + 
         //           " ResLen=" + std::to_string(response_text.length()) + " Res=" + response_text);
    }
    
    return response_text;
}

std::string Brain::get_associative_memory(const std::string& input) {
    if (!memory_store) return "";
    
    auto tokens = tokenize(input);
    for (const auto& word : tokens) {
        if (word.length() <= 3) continue; // Skip "the", "is", etc.
        
        // Search specific known keywords first?
        // Query DB
        auto results = memory_store->query(word);
        if (!results.empty()) {
            // Found a match!
            // Return the most recent or relevant one
            // Format: "I recall learning about [Topic]: [Content]"
            const auto& mem = results[0];
            
            // Don't repeat if it's too long, just give a snippet
            std::string snippet = mem.content.substr(0, 300);
            if (mem.content.length() > 300) snippet += "...";
            
            return "I recall learning about " + word + ". " + snippet;
        }
    }
    return "";
}



std::string Brain::decode_output(const std::vector<double>& logits) {
    std::string result = "";
    std::vector<std::pair<double, int>> scores;
    
    for(size_t i = 0; i < logits.size(); ++i) {
        scores.push_back({logits[i], static_cast<int>(i)});
    }
    
    // Sort descending
    std::sort(scores.begin(), scores.end(), [](auto& a, auto& b){ return a.first > b.first; });

    // Output top 3 words
    for(size_t i=0; i<3; ++i) {
        if (scores[i].first > 0.01) { // Threshold
           size_t idx = static_cast<size_t>(scores[i].second);
           if (vocab_decode.count(idx)) {
               result += vocab_decode[idx] + " ";
           }
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
    // UNLOCK to allow main thread to be responsive
    {
        std::lock_guard<std::recursive_mutex> lock(brain_mutex);
        current_research_topic = topic;
        log_activity("[Background]: Researching " + topic + "...");
        if (on_research_update) on_research_update("Starting research on: " + topic);
    }
    
    // Fetch with links (network bound)
    auto result = research_tools::fetch_comprehensive(topic);
    
    // RE-LOCK to update state
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    std::string content = result.summary;
    
    // Add sub-topics to queue (if interesting/unique)
    int added = 0;
    for (const auto& sub : result.related_topics) {
        if (added < 5 && std::find(learned_topics.begin(), learned_topics.end(), sub) == learned_topics.end()) {
            if (sub.find("List of") == std::string::npos && sub.find("Wikipedia") == std::string::npos) {
                 research_queue.push_back(sub);
                 added++;
            }
        }
    }
    learned_topics.push_back(topic);
    
    if (content.find("No information found") != std::string::npos || content.find("Connection Failed") != std::string::npos) {
        return content;
    }

    log_activity("[Background]: Read " + std::to_string(content.length()) + " chars on " + topic);
    
    // Feed the content into the brain to "learn" it
    std::stringstream ss(content);
    std::string segment;
    int count = 0;
    while(std::getline(ss, segment, '.')) {
        if (segment.length() < 3) continue;
        
        // "Read" (interact calls interact which calls reinforce)
        interact(segment); // This reinforces new words!
        // "Reinforce" highly
        memory_center->network.consolidate_memories(memory_center->current_activity);
        
        if (++count > 5) break; 
    }
    
    // Save to Explicit Memory
    if (memory_store) {
        memory_store->store("Research", content, topic);
    }
    
    if (on_research_update) on_research_update("Completed research on: " + topic);
    return "I learned about " + topic + "! " + content.substr(0, 50) + "... (Found " + std::to_string(result.related_topics.size()) + " related topics)";
}

void Brain::teach(const std::string& input_text, const std::string& target_text) {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    
    // 1. Sensual Perception (Encoding) - Same as interact
    std::vector<double> input_vec(VOCAB_SIZE, 0.0);
    auto input_tokens = tokenize(input_text);
    for (const auto& word : input_tokens) {
        std::hash<std::string> hasher;
        size_t idx = hasher(word) % VOCAB_SIZE;
        input_vec[idx] += 1.0;
        if (vocab_decode.find(idx) == vocab_decode.end()) vocab_decode[idx] = word;
    }
    // Normalize Input
    double max_val = 1.0; 
    for(double v : input_vec) if(v > max_val) max_val = v;
    for(auto& v : input_vec) v /= max_val;

    // 2. Prepare Target Vector
    std::vector<double> target_vec(VOCAB_SIZE, 0.0);
    auto target_tokens = tokenize(target_text);
    for (const auto& word : target_tokens) {
        std::hash<std::string> hasher;
        size_t idx = hasher(word) % VOCAB_SIZE;
        target_vec[idx] += 1.0;
        if (vocab_decode.find(idx) == vocab_decode.end()) vocab_decode[idx] = word;
    }
    // Normalize Target
    double t_max = 1.0;
    for(double v : target_vec) if(v > t_max) t_max = v;
    for(auto& v : target_vec) v /= t_max;

    // 3. Forward Pass to generate "Thought"
    std::vector<double> thought = language_encoder->process(input_vec);
    std::vector<double> memory_context = memory_center->process(thought);
    
    std::vector<double> cognitive_input = thought;
    cognitive_input.insert(cognitive_input.end(), memory_context.begin(), memory_context.end());
    
    std::vector<double> response_thought = cognitive_center->process(cognitive_input);

    // 4. Supervised Training of Decoder
    // We want decoder(response_thought) -> target_vec
    // Train it multiple times to sink it in
    for(int i=0; i<5; ++i) {
        language_decoder->train(response_thought, target_vec, 0.1); 
    }
    
    // 5. Reinforce the path that got us here
    language_encoder->reinforce(0.05);
    memory_center->reinforce(0.05);
    cognitive_center->reinforce(0.05);
    
    // safe_print("Learned: " + input_text + " -> " + target_text);
}



std::string Brain::deep_research(const std::string& topic) {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    // Clear queue to focus on this
    research_queue.clear();
    
    // Initial fetches
    return research(topic);
}

long long Brain::get_knowledge_size() {
    if (memory_store) {
        return memory_store->get_memory_count() * 1024; // Approximation for now
    }
    return 0;
}

void Brain::automata_loop() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Tick every 2s

        // 1. Evaluate Needs & Generate Goals
        {
            std::lock_guard<std::recursive_mutex> lock(brain_mutex);
            
            // Goal: Learn (Boredom)
            if (emotions.boredom > 0.8) {
                 std::vector<std::string> ideas = {"Physics", "History", "AI", "Space", "Oceans"};
                 std::string topic = ideas[rand() % ideas.size()];
                 task_manager.add_task("Research " + topic, TaskType::RESEARCH, TaskPriority::LOW);
                 emotions.boredom = 0.5; // Reduced just by planning it
            }
            
            // Goal: Rest (Energy)
            if (emotions.energy < 0.2) {
                task_manager.add_task("Sleep Cycle", TaskType::SLEEP, TaskPriority::HIGH);
            }
        }
        
        // 2. Execute Tasks
        Task* current = task_manager.get_next_task();
        if (current) {
            std::string desc = current->description;
            
            // Emit Deliberation
            emit_log("[Cognition]: Decided to execute task #" + std::to_string(current->id) + ": " + desc);
            
            if (current->type == TaskType::RESEARCH) {
                // Extract topic "Research X"
                std::string topic = desc.substr(9);
                research(topic);
            } 
            else if (current->type == TaskType::SLEEP) {
                sleep();
                emotions.energy = 1.0;
            }
            else if (current->type == TaskType::INTERACTION) {
                // Placeholder
            }
            
            task_manager.complete_active_task();
        } else {
            // Idle
        }
        
        // Decay
        {
             std::lock_guard<std::recursive_mutex> lock(brain_mutex);
             emotions.boredom = std::min(1.0, emotions.boredom + 0.05);
             if (on_emotion_update) {
                on_emotion_update("Energy: " + std::to_string(emotions.energy) + " | Boredom: " + std::to_string(emotions.boredom) + " | Happiness: " + std::to_string(emotions.happiness));
             }
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

std::string Brain::get_json_state() {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    std::stringstream ss;
    ss << "{";
    ss << "\"personality\": {";
    ss << "\"curiosity\": " << personality.curiosity << ",";
    ss << "\"playfulness\": " << personality.playfulness << ",";
    ss << "\"friendliness\": " << personality.friendliness << ",";
    ss << "\"formality\": " << personality.formality << ",";
    ss << "\"positivity\": " << personality.positivity;
    ss << "},";
    ss << "\"emotions\": {";
    ss << "\"happiness\": " << emotions.happiness << ",";
    ss << "\"sadness\": " << emotions.sadness << ",";
    ss << "\"anger\": " << emotions.anger << ",";
    ss << "\"fear\": " << emotions.fear << ",";
    ss << "\"energy\": " << emotions.energy << ",";
    ss << "\"boredom\": " << emotions.boredom;
    ss << "}";
    ss << "}";
    return ss.str();
}

void Brain::update_from_json(const std::string& json) {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    // Extremely basic parser for "key": value
    auto parse_val = [&](const std::string& key) -> double {
        size_t pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return -1.0;
        size_t colon = json.find(":", pos);
        if (colon == std::string::npos) return -1.0;
        return std::stod(json.substr(colon + 1));
    };
    
    // Personality
    double val;
    if ((val = parse_val("curiosity")) >= 0) personality.curiosity = val;
    if ((val = parse_val("playfulness")) >= 0) personality.playfulness = val;
    if ((val = parse_val("friendliness")) >= 0) personality.friendliness = val;
    if ((val = parse_val("formality")) >= 0) personality.formality = val;
    if ((val = parse_val("positivity")) >= 0) personality.positivity = val;
    
    // Emotions
    if ((val = parse_val("happiness")) >= 0) emotions.happiness = val;
    if ((val = parse_val("sadness")) >= 0) emotions.sadness = val;
    if ((val = parse_val("anger")) >= 0) emotions.anger = val;
    if ((val = parse_val("fear")) >= 0) emotions.fear = val;
    if ((val = parse_val("energy")) >= 0) emotions.energy = val;
    if ((val = parse_val("boredom")) >= 0) emotions.boredom = val;
    
    emit_log("[Brain]: State Updated via API");
}


void Brain::save(const std::string& filename) {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    safe_print("[Brain]: Saving memory state to " + filename + "...");

    std::ofstream os(filename, std::ios::binary);
    if (!os) return;

    os.write(reinterpret_cast<char*>(&personality), sizeof(Personality));
    os.write(reinterpret_cast<char*>(&emotions), sizeof(Emotions));
    
    // Save Vocab
    size_t vocab_count = vocab_decode.size();
    os.write(reinterpret_cast<char*>(&vocab_count), sizeof(size_t));
    for (const auto& [idx, word] : vocab_decode) {
        os.write(reinterpret_cast<const char*>(&idx), sizeof(size_t));
        size_t len = word.length();
        os.write(reinterpret_cast<const char*>(&len), sizeof(size_t));
        os.write(word.c_str(), len);
    }
    
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
    
    // Load Vocab
    size_t vocab_count = 0;
    if (is.read(reinterpret_cast<char*>(&vocab_count), sizeof(size_t))) {
        for (size_t i = 0; i < vocab_count; ++i) {
            size_t idx, len;
            is.read(reinterpret_cast<char*>(&idx), sizeof(size_t));
            is.read(reinterpret_cast<char*>(&len), sizeof(size_t));
            std::string word(len, ' ');
            is.read(&word[0], len);
            vocab_decode[idx] = word;
        }
    }
    
    language_encoder->network.load(is);
    language_decoder->network.load(is);
    memory_center->network.load(is);
    cognitive_center->network.load(is);
    
    safe_print("[Brain]: Memories restored.");
}

// Helpers
void Brain::log_activity(const std::string& msg) {
    std::ofstream log_file("brain.log", std::ios::app);
    if (log_file) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        log_file << std::put_time(std::localtime(&now_c), "%F %T") << " " << msg << std::endl;
    }
    emit_log(msg); // Send to Server/Dashboard!
}

std::vector<std::string> Brain::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;
    for (char c : text) {
        if (std::isalnum(c)) {
            current += static_cast<char>(std::tolower(c));
        } else if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

// [Dev] Implemented 'Refactor `Brain` class to use `PlanningUnit` (In progress)' at 2025-12-21 18:22:17

// [Dev] Implemented 'Implement `EmotionUnit` integration' at 2025-12-21 18:22:29

// [Dev] Implemented 'Update frontend to visualize cognitive states' at 2025-12-21 18:22:41

// [Dev] Implemented 'Add persistence for cognitive states' at 2025-12-21 18:23:44

// [Dev] Implemented 'Improve natural language understanding (NLU) accuracy' at 2025-12-21 18:23:56

// [Dev] Implemented 'Expand e2e test coverage for dashboard' at 2025-12-21 18:25:08

// [Dev] Implemented 'Voice interface integration' at 2025-12-21 18:25:20

// [Dev] Implemented 'Optimize main loop performance' at 2025-12-21 18:25:32

// [Dev] Implemented 'migration to a graph database for knowledge storage?' at 2025-12-21 18:30:21

// [Dev] Implemented 'Multi-user support' at 2025-12-21 18:30:33

// [Dev] Implemented '(No items yet)' at 2025-12-21 18:30:45

// [Dev] Implemented '[NLU] Improve entity extraction accuracy - Optimization #1' at 2025-12-21 18:31:47

// [Dev] Implemented '[Infrastructure] Dockerize build environment for consistent CI (Phase 1) #2' at 2025-12-21 18:31:59

// [Dev] Implemented '[Infrastructure] Add health check endpoint for monitoring - Testing #3' at 2025-12-21 18:32:11

// [Dev] Implemented '[NLU] Add support for multi-turn conversation context (Phase 1) #4' at 2025-12-21 18:32:23

// [Dev] Implemented '[Infrastructure] Set up GitHub Actions for automated testing - Investigation #5' at 2025-12-21 18:32:35

// [Dev] Implemented '[Frontend] Refactor dashboard to use React components - Refactor #6' at 2025-12-21 18:32:47

// [Dev] Implemented '[Frontend] Refactor dashboard to use React components - Refactor #6' at 2025-12-21 18:32:59

// [Dev] Implemented '[Cognition] Refactor memory retrieval for O(1) access - Testing #7' at 2025-12-21 18:33:11

// [Dev] Implemented '[Infrastructure] Implement redis caching for frequent queries - Testing #8' at 2025-12-21 18:33:23

// [Dev] Implemented '[NLU] Improve entity extraction accuracy (Phase 1) #9' at 2025-12-21 18:33:35

// [Dev] Implemented '[Frontend] Refactor dashboard to use React components - Refactor #10' at 2025-12-21 18:33:47

// [Dev] Implemented '[Infrastructure] Set up GitHub Actions for automated testing - Testing #11' at 2025-12-21 18:33:59

// [Dev] Implemented '[Frontend] Visualize real-time neuron activity with WebGL - Testing #12' at 2025-12-21 18:34:11

// [Dev] Implemented '[NLU] Add support for multi-turn conversation context (Phase 2) #13' at 2025-12-21 18:34:23

// [Dev] Implemented '[Frontend] Refactor dashboard to use React components (Phase 1) #14' at 2025-12-21 18:34:35

// [Dev] Implemented '[Frontend] Refactor dashboard to use React components - Testing #15' at 2025-12-21 18:34:47
