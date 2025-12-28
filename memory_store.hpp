#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <memory>
#include <unordered_map>
#include "postgres_client.hpp"
#include "postgres_storage.hpp"

struct Memory {
    int id;
    long long timestamp;
    std::string type; // "Observation", "Thought", "Research"
    std::string content;
    std::string tags;
    
    // Feature 6 & 10 fields
    std::string acl_label = "PUBLIC"; // PUBLIC, PRIVATE, RESTRICTED
    double strength = 1.0; // Decay factor (Ebbinghaus)
    long long last_recall_time = 0;
};

class MemoryStore {
public:
    MemoryStore(const std::string& conn_str);
    ~MemoryStore();

    bool init();
    bool store(const std::string& type, const std::string& content, const std::string& tags = "", const std::string& acl = "PUBLIC");
    std::vector<Memory> query(const std::string& keyword, const std::string& user_acl = "PUBLIC");
    std::vector<Memory> get_recent(int limit = 10);
    long long get_memory_count();
    std::string get_graph_json(int max_nodes = 50);
    void clear();
    
    // Mega-Batch 8: Vector Support
    void store_embedding(const std::string& key, const std::vector<double>& embedding);
    std::vector<double> retrieve_embedding(const std::string& key);
    std::vector<std::string> search_similar(const std::vector<double>& embedding, int limit);

private:
    std::unique_ptr<PostgresClient> pg_client;
    std::unique_ptr<PostgresStorage> kv_store;
    std::string conn_str_;
    mutable std::mutex db_mutex_;
    // Inverted Index: Token -> List of Memory IDs
    std::unordered_map<std::string, std::vector<int>> inverted_index_;

    void build_index();
    void index_memory(int id, const std::string& content);
};
