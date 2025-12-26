#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <memory>
#include <unordered_map>
#include "postgres_client.hpp"

struct Memory {
    int id;
    long long timestamp;
    std::string type; // "Observation", "Thought", "Research"
    std::string content;
    std::string tags;
};

class MemoryStore {
public:
    MemoryStore(const std::string& conn_str);
    ~MemoryStore();

    bool init();
    bool store(const std::string& type, const std::string& content, const std::string& tags = "");
    std::vector<Memory> query(const std::string& keyword);
    std::vector<Memory> get_recent(int limit = 10);
    long long get_memory_count();
    std::string get_graph_json(int max_nodes = 50);
    void clear();

private:
    std::unique_ptr<PostgresClient> pg_client;
    std::string conn_str_;
    mutable std::mutex db_mutex_;
    // Inverted Index: Token -> List of Memory IDs
    std::unordered_map<std::string, std::vector<int>> inverted_index_;

    void build_index();
    void index_memory(int id, const std::string& content);
};
