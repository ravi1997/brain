#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>
#include <iostream>
#include <mutex>

struct Memory {
    int id;
    long long timestamp;
    std::string type; // "Observation", "Thought", "Research"
    std::string content;
    std::string tags;
};

#include <unordered_map>

class MemoryStore {
public:
    MemoryStore(const std::string& db_path);
    ~MemoryStore();

    bool init();
    bool store(const std::string& type, const std::string& content, const std::string& tags = "");
    std::vector<Memory> query(const std::string& keyword);
    std::vector<Memory> get_recent(int limit = 10);
    long long get_memory_count();

private:
    sqlite3* db_ = nullptr;
    std::string db_path_;
    mutable std::mutex db_mutex_;
    // Inverted Index: Token -> List of Memory IDs
    std::unordered_map<std::string, std::vector<int>> inverted_index_;

    bool execute(const std::string& sql);
    void build_index();
    void index_memory(int id, const std::string& content);
};
