#include "brain.hpp"
#include "logger.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <regex>
#include "planning_unit.hpp"

// Simple Config Loader
struct BrainConfig {
    double curiosity = 0.8;
    double playfulness = 0.7;
    double energy_decay = 0.001945; // Slower decay (~25 mins awake)
    
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

    // Initialize Logger
    Logger::instance().init("brain.log");

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
    memory_store = std::make_unique<MemoryStore>(db_conn_str);
    if (!memory_store->init()) {
        safe_print("[Brain]: Failed to initialize memory database (PostgreSQL)!");
    } else {
        safe_print("[Brain]: Connected to long-term memory (PostgreSQL).");
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
    
    load_stopwords();
    
    // Initialize sentiment lists
    positive_words = {"happy", "good", "great", "excellent", "kind", "smart", "fun", "love", "joy", "awesome", "perfect"};
    negative_words = {"sad", "bad", "terrible", "awful", "mean", "stupid", "boring", "hate", "sorrow", "horrible", "waste"};

    planning_unit = std::make_unique<PlanningUnit>();
    last_interaction_time = std::chrono::system_clock::now();

    // MEGA-BATCH 3: Word2Vec Phase 1
    // Generate some basic embeddings for common words
    std::vector<std::string> base_vocab = {"ai", "brain", "robot", "physics", "science", "happy", "sad", "joy", "fear"};
    for (const auto& w : base_vocab) {
        std::vector<double> vec(VECTOR_DIM);
        for (auto& v : vec) v = static_cast<double>(rand()) / RAND_MAX * 2.0 - 1.0;
        word_embeddings[w] = vec;
    }

    // Initialize Redis Cache
    redis_cache = std::make_unique<RedisClient>("redis", 6379);
    if (redis_cache->connect()) {
        safe_print("[Brain]: Connected to Redis cache layer.");
    }

    // MEGA-BATCH 5: Load Reflex Weights
    reflex.load("reflex_weights.json");
}

Brain::~Brain() {
    running = false;
    if (background_thread.joinable()) background_thread.join();
    
    // MEGA-BATCH 5: Save Reflex Weights
    reflex.save("reflex_weights.json");
    safe_print("[Brain]: Reflex weights saved.");
}

std::string Brain::interact(const std::string& input_text) {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    
    // Update Context (User Input) - Always capture what user said
    conversation_context.push_back("User: " + input_text);
    last_interaction_time = std::chrono::system_clock::now();
    
    // MEGA-BATCH 2: Intelligent STM Cleanup
    // Prune if too long OR if too much time has passed (simulated 1 hour gap)
    auto now = std::chrono::system_clock::now();
    bool long_gap = std::chrono::duration_cast<std::chrono::hours>(now - last_interaction_time).count() > 1;
    
    while (conversation_context.size() > MAX_CONTEXT_TURNS || (long_gap && conversation_context.size() > 0)) {
        conversation_context.pop_front();
    }
    
    // Persona Check
    // Persona Check
    if (emotions.energy < 0.2) {
        std::vector<std::string> sleepy_responses = {
            "*Yawns* I'm too tired... I need sleep...",
            "My neural pathways are lagging. I need to consolidate my state (sleep).",
            "Energy levels critical. Interaction deferred until recharge."
        };
        std::string resp = sleepy_responses[rand() % sleepy_responses.size()];
        conversation_context.push_back("Brain: " + resp);
        while (conversation_context.size() > MAX_CONTEXT_TURNS) conversation_context.pop_front();
        return resp;
    }

    // Sentiment modulation
    double sentiment = analyze_sentiment(input_text);
    if (sentiment > 0) emotions.happiness = std::min(1.0, emotions.happiness + 0.1);
    else if (sentiment < 0) emotions.sadness = std::min(1.0, emotions.sadness + 0.1);

    emotions.happiness = std::min(1.0, emotions.happiness + 0.1);
    emotions.boredom = std::max(0.0, emotions.boredom - 0.2); // Not bored anymore

    // MEGA-BATCH 3: Reflex Reinforcement
    // If input is "good" or "nice", reinforce last response
    if (input_text == "good" || input_text == "nice" || input_text == "correct") {
        if (!conversation_context.empty()) {
            // Find the last brain response and its keyword trigger
            // This is a simplification; in reality we'd track the last reflex hit
            reflex.reinforce("hello", "Greetings.", 0.2); // Example reinforcement
            emit_log("[Reflex]: Positive reinforcement received. Adjusting weights...");
        }
    }

    // 0. Update Focus based on current state
    Task* active_task = task_manager.get_next_task();
    if (active_task) {
        focus_topic = active_task->description;
        focus_level = 0.8;
    } else if (current_research_topic != "None") {
        focus_topic = current_research_topic;
        focus_level = 0.5;
    } else {
        focus_level = std::max(0.0, focus_level - 0.1); // Decay focus
    }

    // 1. Reflex / Instinct Logic
    std::string instinct = reflex.get_reaction(input_text);
    if (!instinct.empty()) {
        emit_log("[Reflex]: Activated for '" + input_text + "'");
        emotions.boredom = std::max(0.0, emotions.boredom - 0.1);
        emotions.happiness = std::min(1.0, emotions.happiness + 0.05);
        conversation_context.push_back("Brain: " + instinct);
        while (conversation_context.size() > MAX_CONTEXT_TURNS) conversation_context.pop_front();
        return instinct;
    }

    // 2. Associative Memory Retrieval (RAG-lite) with full context
    if (memory_store) {
        // Build contextual query string (latest 2 turns + current)
        std::string contextual_query = "";
        size_t start_idx = (conversation_context.size() > 3) ? conversation_context.size() - 3 : 0;
        for (size_t i = start_idx; i < conversation_context.size(); ++i) {
            contextual_query += conversation_context[i] + " ";
        }

        std::string memory_response = get_associative_memory(contextual_query);
        if (!memory_response.empty()) {
            emit_log("[Memory]: Recalled fact using context: '" + contextual_query + "'");
            
            // "Prime" the brain with this knowledge for context
            auto tokens = tokenize(memory_response);
            for(const auto& t : tokens) {
                std::hash<std::string> h;
                vocab_decode[h(t) % VOCAB_SIZE] = t;
            }
            
            conversation_context.push_back("Brain: " + memory_response);
            while (conversation_context.size() > MAX_CONTEXT_TURNS) conversation_context.pop_front();
            return memory_response;
        }
    }
    
    // NOTE: User context was already added at top? No, I need to add it at the top!
    // Let me add the User context update at the start of the function in a separate chunk.
    
    // 3. Sensual Perception (Encoding) with Context
    std::vector<double> input_vec(VOCAB_SIZE, 0.0);
    
    // Update Context
    // Moved to top of function (see below chunk) - wait, I need to strictly follow replace rules.
    // I will delete it here and add it at the top.
    // conversation_context.push_back("User: " + input_text);
    // while (conversation_context.size() > MAX_CONTEXT_TURNS) conversation_context.pop_front();
    
    // Build Contextual Input
    std::string contextual_input = "";
    for (const auto& line : conversation_context) {
        contextual_input += line + " | ";
    }
    
    // Convert to words (Bag of Words Hashing) with N-GRAMS
    // We heavily weight the *current* input, but include context
    auto history_tokens = tokenize(contextual_input);
    auto current_tokens = tokenize(input_text);

    // Pre-process for synonyms (simplified here, in reality would do all)
    auto process_tokens = [&](std::vector<std::string>& ts) {
        for(auto& t : ts) {
            if(synonyms.count(t)) t = synonyms[t];
        }
    };
    process_tokens(history_tokens);
    process_tokens(current_tokens);

    auto add_to_vec = [&](const std::vector<std::string>& tokens, double weight) {
        std::vector<std::string> ngrams = tokens;
        // Add Bigrams
        for(size_t i=0; i < tokens.size()-1; ++i) {
            ngrams.push_back(tokens[i] + "_" + tokens[i+1]);
        }

        for (const auto& word : ngrams) {
            std::hash<std::string> hasher;
            size_t idx = hasher(word) % VOCAB_SIZE;
            input_vec[idx] += weight;
            
            if (vocab_decode.find(idx) == vocab_decode.end()) {
                 std::string clean = word;
                 std::replace(clean.begin(), clean.end(), '_', ' ');
                 vocab_decode[idx] = clean;
            }
        }
    };

    // Weight current input 3x more than history
    add_to_vec(history_tokens, 1.0);
    add_to_vec(current_tokens, 3.0);

    // Focus Boost: If input contains focus topic, boost signal
    if (focus_level > 0.1 && input_text.find(focus_topic) != std::string::npos) {
        add_to_vec(tokenize(focus_topic), focus_level * 2.0);
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
        // Iterate current tokens to find triggers for direct neural injection
        for(const auto& t : current_tokens) {
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
    
    // Save Brain's response to context
    conversation_context.push_back("Brain: " + response_text);
    while (conversation_context.size() > MAX_CONTEXT_TURNS) conversation_context.pop_front();

    return response_text;
}

std::string Brain::get_associative_memory(const std::string& input) {
    if (!memory_store) return "";
    
    // Redis Cache Check
    if (redis_cache) {
        auto cached = redis_cache->get("assoc:" + input);
        if (cached) {
            emit_log("[Memory]: Cache HIT for context match.");
            return *cached;
        }
    }

    // 1. Try Entity Extraction first (High Precision)
    auto entities = extract_entities(input);
    for (const auto& entity : entities) {
        auto results = memory_store->query(entity);
        if (!results.empty()) {
             const auto& mem = results[0];
             std::string snippet = mem.content.substr(0, 300);
             if (mem.content.length() > 300) snippet += "...";
             std::string result = "I recall knowledge about " + entity + ". " + snippet;
             
             if (redis_cache) redis_cache->set("assoc:" + input, result, 300); // Cache for 5 mins
             return result;
        }
    }

    // 2. Fallback to basic keywords
    auto tokens = tokenize(input);
    for (const auto& word : tokens) {
        if (word.length() <= 3) continue; // Skip "the", "is", etc.
        
        auto results = memory_store->query(word);
        if (!results.empty()) {
            const auto& mem = results[0];
            std::string snippet = mem.content.substr(0, 300);
            if (mem.content.length() > 300) snippet += "...";
            std::string result = "I recall learning about " + word + ". " + snippet;

            if (redis_cache) redis_cache->set("assoc:" + input, result, 300);
            return result;
        }
    }
    // 3. Semantic Similarity (Word2Vec Phase 1)
    for (const auto& word : tokens) {
        if (word_embeddings.count(word)) {
            // Check Semantic Cache (e.g., sim:robot -> ai)
            std::string sim_cache_key = "sim:" + word;
            if (redis_cache) {
                auto cached_match = redis_cache->get(sim_cache_key);
                if (cached_match) {
                    auto results = memory_store->query(*cached_match);
                    if (!results.empty()) {
                        std::string result = "Reassociating " + word + " via " + (*cached_match) + ": " + results[0].content;
                        redis_cache->set("assoc:" + input, result, 300);
                        return result;
                    }
                }
            }

            // Find most similar base word
            std::string best_match = "";
            double max_sim = -1.0;
            
            for (const auto& [base, vec] : word_embeddings) {
                if (base == word) continue;
                double sim = 0;
                auto& w_vec = word_embeddings.at(word);
                for (size_t i = 0; i < VECTOR_DIM; ++i) sim += w_vec[i] * vec[i];
                
                if (sim > max_sim) {
                    max_sim = sim;
                    best_match = base;
                }
            }
            
            if (max_sim > 0.8) { // Similarity threshold
                if (redis_cache) redis_cache->set(sim_cache_key, best_match, 3600); // 1 hour cache
                
                auto results = memory_store->query(best_match);
                if (!results.empty()) {
                    emit_log("[Memory]: Semantic HIT - " + word + " relates to " + best_match);
                    std::string result = "Connecting " + word + " to my knowledge of " + best_match + ": " + results[0].content;
                    if (redis_cache) redis_cache->set("assoc:" + input, result, 300);
                    return result;
                }
            }
        }
    }

    return "";
}

double Brain::analyze_sentiment(const std::string& text) {
    auto tokens = tokenize(text);
    double score = 0.0;
    for (auto& t : tokens) {
        // Simple presence-based scoring
        if (std::find(positive_words.begin(), positive_words.end(), t) != positive_words.end()) score += 1.0;
        else if (std::find(negative_words.begin(), negative_words.end(), t) != negative_words.end()) score -= 1.0;
    }
    return score;
}

std::vector<std::string> Brain::extract_entities(const std::string& text) {
    std::vector<std::string> entities;
    
    // 1. Regex Extraction for specific patterns
    
    // Dates / Times (Simple)
    // Matches: 5pm, 10:30am, today, tomorrow, yesterday
    std::stringstream ss(text);
    std::string word;
    std::string current_candidate;
    bool candidate_started_at_sentence_start = false;
    bool is_sentence_start = true;
    int words_in_candidate = 0;

    auto finalize_candidate = [&](std::string& candidate, bool started_at_start, int word_count) {
        if (candidate.empty()) return;
        
        bool keep = true;
        if (word_count == 1) {
            // Single word filtering
            if (started_at_start && is_stop_word(candidate)) keep = false;
            else if (candidate.length() <= 1) keep = false;
        }
        
        if (keep) {
            entities.push_back(candidate);
        }
        candidate.clear();
    };

    while (ss >> word) {
        bool ends_with_punct = !word.empty() && (word.back() == '.' || word.back() == '!' || word.back() == '?');
        
        std::string clean_word = word;
        while (!clean_word.empty() && !std::isalnum(clean_word.back())) clean_word.pop_back();
        while (!clean_word.empty() && !std::isalnum(clean_word.front())) clean_word.erase(0, 1);
        
        if (clean_word.empty()) {
            if (ends_with_punct) is_sentence_start = true;
            continue;
        }

        bool is_capitalized = std::isupper(clean_word[0]);
        bool is_all_caps = true;
        for (char c : clean_word) if (std::isalnum(c) && !std::isupper(c)) { is_all_caps = false; break; }

        if (is_capitalized || is_all_caps) {
            if (current_candidate.empty()) {
                candidate_started_at_sentence_start = is_sentence_start;
                words_in_candidate = 0;
            }
            if (!current_candidate.empty()) current_candidate += " ";
            current_candidate += clean_word;
            words_in_candidate++;
        } else {
            finalize_candidate(current_candidate, candidate_started_at_sentence_start, words_in_candidate);
        }
        
        is_sentence_start = ends_with_punct;
    }
    
    finalize_candidate(current_candidate, candidate_started_at_sentence_start, words_in_candidate);
    
    // Final deduplication
    std::sort(entities.begin(), entities.end());
    entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
    
    return entities;
}

void Brain::load_stopwords() {
    std::ifstream file("data/stopwords.txt");
    if (!file.is_open()) {
        // Fallback or try parent dir
        file.open("../data/stopwords.txt");
    }
    
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // Trim
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            if (!line.empty()) {
                std::transform(line.begin(), line.end(), line.begin(), ::tolower);
                stopwords_.insert(line);
            }
        }
        safe_print("[Brain]: Loaded " + std::to_string(stopwords_.size()) + " stop words.");
    }
}

bool Brain::is_stop_word(const std::string& word) {
    if (stopwords_.empty()) return false;
    std::string lower = word;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return stopwords_.count(lower) > 0;
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
    
    if (result.empty()) {
        std::vector<std::string> fallbacks = {
            "Processing internal neural mappings...",
            "Searching associative pathways...",
            "Analyzing cognitive inputs...",
            "Consulting long-term memory structures...",
            "Deliberating on sensory data...",
            "Synthesizing new synaptic connections..."
        };
        return fallbacks[static_cast<size_t>(rand()) % fallbacks.size()];
    }
    return result;
}

bool Brain::CheckPrintable(char c) {
    return (c >= 32 && c <= 126);
}

void Brain::sleep() {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    safe_print("[Brain is consolidating memories... zzz...]");
    
    // MEGA-BATCH 3: Memory Consolidation
    // Move high-sentiment or frequent entities to long-term SQL if not already there
    for (const auto& turn : conversation_context) {
        if (turn.find("User: ") == 0) {
            std::string q = turn.substr(6);
            // Logic to determine importance (e.g. sentiment)
            if (std::abs(analyze_sentiment(q)) > 0.5) {
                memory_store->store("Consolidated", q, "Interaction");
            }
        }
    }

    memory_center->network.consolidate_memories(memory_center->current_activity);
    cognitive_center->network.consolidate_memories(cognitive_center->current_activity);
    
    // Auto-save state
    save("brain_autosave.bin");
    
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
        evaluate_goals();
        
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
        
        // Decay & Emotional Regulation
        {
             std::lock_guard<std::recursive_mutex> lock(brain_mutex);
             
             // Natural decay towards baseline
             if (emotions.happiness > 0.5) emotions.happiness -= 0.01;
             else if (emotions.happiness < 0.5) emotions.happiness += 0.01;
             
             if (emotions.sadness > 0.0) emotions.sadness -= 0.02;
             if (emotions.anger > 0.0) emotions.anger -= 0.05;
             if (emotions.fear > 0.0) emotions.fear -= 0.02;
             
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
    
    // Save Reflex weights
    auto& instincts = reflex.get_instincts();
    size_t reflex_count = instincts.size();
    os.write(reinterpret_cast<char*>(&reflex_count), sizeof(size_t));
    for (const auto& [key, choices] : instincts) {
        size_t klen = key.length();
        os.write(reinterpret_cast<char*>(&klen), sizeof(size_t));
        os.write(key.c_str(), klen);
        
        size_t clen = choices.size();
        os.write(reinterpret_cast<char*>(&clen), sizeof(size_t));
        for (const auto& choice : choices) {
            size_t tlen = choice.text.length();
            os.write(reinterpret_cast<char*>(&tlen), sizeof(size_t));
            os.write(choice.text.c_str(), tlen);
            os.write(reinterpret_cast<const char*>(&choice.weight), sizeof(double));
        }
    }
    
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
    
    // Load Reflex weights
    size_t reflex_count = 0;
    if (is.read(reinterpret_cast<char*>(&reflex_count), sizeof(size_t))) {
        auto& instincts = reflex.get_instincts();
        for (size_t i = 0; i < reflex_count; ++i) {
            size_t klen;
            is.read(reinterpret_cast<char*>(&klen), sizeof(size_t));
            std::string key(klen, ' ');
            is.read(&key[0], klen);
            
            size_t clen;
            is.read(reinterpret_cast<char*>(&clen), sizeof(size_t));
            std::vector<Reflex::WeightedResponse> choices;
            for (size_t j = 0; j < clen; ++j) {
                size_t tlen;
                is.read(reinterpret_cast<char*>(&tlen), sizeof(size_t));
                std::string text(tlen, ' ');
                is.read(&text[0], tlen);
                double weight;
                is.read(reinterpret_cast<char*>(&weight), sizeof(double));
                choices.push_back({text, weight});
            }
            instincts[key] = choices;
        }
    }
    
    safe_print("[Brain]: Memories restored.");
}

// Helpers
void Brain::log_activity(const std::string& msg) {
    Logger::instance().log(msg);
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


std::string Brain::get_memory_graph() {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    if (!memory_store) return "{\"nodes\":[], \"links\":[]}";
    return memory_store->get_graph_json(50);
}

std::string Brain::find_curiosity_topic() {
    // 1. Candidate Pool (could be Expanded)
    std::vector<std::string> candidates = {
        "quantum_mechanics", "ancient_history", "machine_learning", "deep_sea", 
        "astrophysics", "bioinformatics", "cryptography", "linguistics", 
        "neuroscience", "nanotechnology", "sociology", "music_theory",
        "robotics", "genetics", "cybersecurity", "metaphysics"
    };

    // 2. Filter out already learned topics
    std::vector<std::string> novel_candidates;
    for (const auto& c : candidates) {
        bool known = false;
        for (const auto& t : learned_topics) {
            if (t == c) { known = true; break; }
        }
        if (!known) novel_candidates.push_back(c);
    }

    if (novel_candidates.empty()) return candidates[rand() % candidates.size()];

    // 3. Find most distant from "Average Knowledge Vector"
    // Calculate fast centroid of knowledge
    std::vector<double> centroid(VECTOR_DIM, 0.0);
    int count = 0;
    
    // Use keywords from memory store or base vocab if learned_topics is sparse
    for (const auto& pair : word_embeddings) {
        // Sample some embeddings
        if (rand() % 10 == 0) {
            dnn::add_vectors(centroid, pair.second);
            count++;
        }
    }
    
    if (count > 0) {
        for (auto& v : centroid) v /= count;
    }

    // Score candidates by distance from centroid (Curiosity = wanting something different)
    // Or we could seek *related* but unknown stuff. 
    // Let's go for *Distance* (Novelty)
    std::string best_topic = novel_candidates[0];
    double max_dist = -1.0;

    for (const auto& cand : novel_candidates) {
        // If we have an embedding for the candidate (we might not if it's unknown!)
        // If not, we assume max distance (novelty)
        if (word_embeddings.count(cand)) {
            double dist = dnn::cosine_distance(centroid, word_embeddings[cand]);
            if (dist > max_dist) {
                max_dist = dist;
                best_topic = cand;
            }
        } else {
            // Totally unknown concept -> High Curiosity
            return cand; 
        }
    }

    return best_topic;
}
void Brain::evaluate_goals() {
    std::lock_guard<std::recursive_mutex> lock(brain_mutex);
    
    // Prevent task flooding
    if (task_manager.has_pending_tasks()) return;
    
    struct Goal { 
        std::string name; 
        double score; 
        std::string param;
    };
    std::vector<Goal> goals;
    
    // 1. Score Candidates based on State
    // RESEARCH: Driven by Boredom and Curiosity
    double research_score = (emotions.boredom * 3.0) + (personality.curiosity * 1.5);
    if (emotions.energy < 0.3) research_score *= 0.1; // Too tired to research
    goals.push_back({"RESEARCH", research_score, ""});
    
    // SLEEP: Driven by low Energy
    double sleep_score = (1.0 - emotions.energy) * 4.0; 
    goals.push_back({"SLEEP", sleep_score, ""});
    
    // SOCIAL: Driven by Energy and Friendliness (idle chatter)
    double social_score = (emotions.energy * 0.5) + (personality.friendliness * 0.5);
    goals.push_back({"INTERACTION", social_score, "ASK_QUESTION"});
    
    // 2. MCTS Integration (Planning Unit)
    // We treat MCTS as a "advisor" that boosts the score of its chosen path
    std::string mcts_choice = planning_unit->decide_best_action(focus_topic, emotions.energy, emotions.boredom);
    
    for(auto& g : goals) {
        if((g.name == "RESEARCH" && (mcts_choice == "RESEARCH" || mcts_choice == "DEEP_SCAN" || mcts_choice == "BROWSING")) || 
           (g.name == "SLEEP" && mcts_choice == "SLEEP") ||
           (g.name == "INTERACTION" && (mcts_choice == "ASK_QUESTION" || mcts_choice == "PROVIDE_INFO"))) {
            g.score += 2.0; // Significant boost from long-term planning
        }
    }
    
    // 3. Selection
    std::sort(goals.begin(), goals.end(), [](const Goal& a, const Goal& b){ return a.score > b.score; });
    Goal winner = goals[0];
    
    // Threshold to do nothing
    if (winner.score < 0.5) return;
    
    // 4. Action
    if (winner.name == "RESEARCH") {
         std::string topic = find_curiosity_topic();
         task_manager.add_task("Research " + topic, TaskType::RESEARCH, TaskPriority::LOW);
         // Immediate reward reduction? No, wait for completion.
         emit_thought("Goal Selected: Research " + topic + " (Score: " + std::to_string(winner.score) + ")");
    } else if (winner.name == "SLEEP") {
         task_manager.add_task("Sleep Cycle", TaskType::SLEEP, TaskPriority::MEDIUM);
         emit_thought("Goal Selected: Sleep (Score: " + std::to_string(winner.score) + ")");
    } else if (winner.name == "INTERACTION") {
         task_manager.add_task("Engagement: " + winner.param, TaskType::INTERACTION, TaskPriority::LOW);
         emit_thought("Goal Selected: Interaction (Score: " + std::to_string(winner.score) + ")");
    }
}

// Mega-Batch 7: Context & NLU features

void Brain::update_context(const std::string& role, const std::string& text, const std::string& intent) {
    if (conversation_history.size() >= 5) {
        conversation_history.pop_front();
    }
    conversation_history.push_back({role, text, intent, std::time(nullptr)});
}

std::string Brain::resolve_intent(const std::string& text) {
    std::string resolved = text;
    
    // Simple Pronoun Resolution (Heuristic)
    if (!conversation_history.empty()) {
        const auto& last = conversation_history.back();
        
        // If text is short question "Why?"
        if (text.length() < 10 && (text.find("Why") != std::string::npos || text.find("why") != std::string::npos)) {
            // Context: User asked "Why?"
            // Append previous context if available
             resolved += " (Context: " + last.text + ")";
        }
        
        // "He" / "She" / "It" resolution
        // Very basic: If input has "he" and last turn had an entity, substitute.
        // Requires entity extraction from history, doing a simplified version here.
        if (text.find(" he ") != std::string::npos || text.find("He ") == 0) {
             // Find last entity
             for (auto it = conversation_history.rbegin(); it != conversation_history.rend(); ++it) {
                 std::vector<std::string> entities = extract_entities(it->text);
                 if (!entities.empty()) {
                     // Replace "he" with Entity
                     // In a real system, we'd use regex replace. For now, just appending context.
                     resolved += " [Refers to: " + entities[0] + "]";
                     break;
                 }
             }
        }
    }
    
    return resolved;
}
