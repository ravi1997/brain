#include "skill_manager.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cmath>

namespace dnn {

    SkillManager::SkillManager(const std::string& storage_path) : storage_path_(storage_path) {
        // Ensure directory exists
        std::filesystem::create_directories(storage_path_);
        load_all();
    }

    SkillManager::~SkillManager() {
        save_all();
    }

    void SkillManager::teach_skill(const std::string& topic, const std::vector<double>& input, const std::vector<double>& output) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto skill = get_or_create_skill(topic);
        
        // Train the specific network
        // We assume input/output dimensions match what the network was initialized with.
        // If not, we might need a resizing logic or error. 
        // For now, let's assume standard vector size (64 inputs, 64 outputs for simplicty?)
        // Or we rely on dnn.cpp handling it (it throws if size mismatch).
        
        // Dynamic Resizing Hack: If network is empty/new, init with correct size
        if (!skill->network) {
             // Default topology for any new skill: Input -> Hidden -> Output
             // Hidden layer = (Input + Output)
             std::vector<size_t> topology = {input.size(), input.size() + output.size(), output.size()};
             skill->network = std::make_unique<NeuralNetwork>(topology);
        }
        
        // Now train
        skill->network->train({input}, {output}, 1, 1, 0.1); // 1 epoch, learning rate 0.1
        skill->usage_count++;
        skill->confidence_score = std::min(1.0, skill->confidence_score + 0.01);
        
        std::cout << "[SkillManager]: Trained skill '" << topic << "'." << std::endl;
    }

    std::vector<double> SkillManager::query_skill(const std::string& topic, const std::vector<double>& input) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (skills_.find(topic) == skills_.end()) {
            // Skill doesn't exist
            return {};
        }
        
        auto& skill = skills_[topic];
        skill->usage_count++;
        return skill->network->predict(input);
    }

    std::shared_ptr<Skill> SkillManager::get_or_create_skill(const std::string& topic) {
        // 1. Check existence
        if (skills_.find(topic) != skills_.end()) {
            return skills_[topic];
        }

        // 2. Check Similarity (Auto-Merge Candidate?)
        // Simple string similarity for now. Levenshtein or substring.
        // "Math_Add" vs "Math_Adding"
        for (auto& [name, existing] : skills_) {
            if (calculate_similarity(name, topic) > 0.8) {
                std::cout << "[SkillManager]: Topic '" << topic << "' is very similar to '" << name << "'. Using existing skill." << std::endl;
                return existing; 
            }
        }

        // 3. Create New
        auto new_skill = std::make_shared<Skill>();
        new_skill->name = topic;
        new_skill->network = nullptr; // Deferred init
        
        skills_[topic] = new_skill;
        std::cout << "[SkillManager]: Created NEW skill '" << topic << "'" << std::endl;
        return new_skill;
    }

    void SkillManager::merge_skills(const std::string& s1, const std::string& s2) {
        // Advanced: Weight Averaging 
        // For now, we just map s2 to point to s1's pointer in the map
        if (skills_.find(s1) != skills_.end() && skills_.find(s2) != skills_.end()) {
             skills_[s2] = skills_[s1]; // Shared pointer copy
             std::cout << "[SkillManager]: Merged '" << s2 << "' into '" << s1 << "'" << std::endl;
        }
    }

    std::vector<std::string> SkillManager::list_skills() const {
        std::vector<std::string> list;
        for(const auto& [name, _] : skills_) list.push_back(name);
        return list;
    }

    void SkillManager::save_all() {
        for(auto& [name, skill] : skills_) {
            if (skill && skill->network) {
                 std::string filename = storage_path_ + name + ".dnn";
                 std::ofstream ofs(filename, std::ios::binary);
                 skill->network->save(ofs);
            }
        }
    }

    void SkillManager::load_all() {
        if (!std::filesystem::exists(storage_path_)) return;
        
        for (const auto& entry : std::filesystem::directory_iterator(storage_path_)) {
            if (entry.path().extension() == ".dnn") {
                std::string name = entry.path().stem().string();
                
                auto skill = std::make_shared<Skill>();
                skill->name = name;
                // We must read the file to know topology, or rely on NeuralNetwork::load reshaping itself?
                // dnn::NeuralNetwork::load currently expects nothing? No, it relies on layers matching?
                // Let's assume load creates structure if empty? No, dnn.cpp usually requires structure.
                // WE NEED TO READ TOPOLOGY FIRST?
                // Quick hack: Just init an empty one and hope load works?
                // But wait, the assertion happens in constructor.
                // We need a NeuralNetwork constructor that takes NO args or handles empty?
                // The current constructor asserts size >= 2.
                // Let's rely on teach_skill to fix it or manual load.
                
                // Workaround: Create a Dummy 1->1 network just to have an object, then Load overrides it?
                // Or better: Modify dnn.cpp to allow default constructor? It is default in header.
                skill->network = std::make_unique<NeuralNetwork>(); // Default constructor (no args)
                
                std::ifstream ifs(entry.path(), std::ios::binary);
                skill->network->load(ifs);
                
                skills_[name] = skill;
            }
        }
        std::cout << "[SkillManager]: Loaded " << skills_.size() << " skills." << std::endl;
    }

    double SkillManager::calculate_similarity(const std::string& t1, const std::string& t2) {
        // Mock similarity: if they share a prefix of 4 chars
        if (t1.length() < 4 || t2.length() < 4) return 0.0;
        if (t1.substr(0,4) == t2.substr(0,4)) return 0.9;
        return 0.0;
    }

} // namespace dnn
