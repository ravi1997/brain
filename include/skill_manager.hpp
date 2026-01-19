#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include "dnn.hpp"

namespace dnn {

    struct Skill {
        std::string name;
        std::unique_ptr<NeuralNetwork> network;
        int usage_count = 0;
        double confidence_score = 0.5;
        
        // Metadata for future vector similarity
        std::vector<double> topic_embedding;
    };

    class SkillManager {
    public:
        SkillManager(const std::string& storage_path = "data/skills/");
        ~SkillManager();

        // Core API
        void teach_skill(const std::string& topic, const std::vector<double>& input, const std::vector<double>& output);
        std::vector<double> query_skill(const std::string& topic, const std::vector<double>& input);
        
        // Management
        std::vector<std::string> list_skills() const;
        void save_all();
        void load_all();
        
        // Advanced functionality
        void merge_skills(const std::string& source_topic, const std::string& target_topic);

    private:
        std::map<std::string, std::shared_ptr<Skill>> skills_;
        std::string storage_path_;
        std::mutex mutex_;

        // Helpers
        std::shared_ptr<Skill> get_or_create_skill(const std::string& topic);
        double calculate_similarity(const std::string& topic1, const std::string& topic2);
    };

} // namespace dnn
