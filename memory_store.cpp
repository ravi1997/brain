#include "memory_store.hpp"
#include "memory_store.hpp"
#include "redis_client.hpp"
#include <chrono>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

MemoryStore::MemoryStore(const std::string& db_path) : db_path_(db_path) {}

MemoryStore::~MemoryStore() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool MemoryStore::init() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS memories ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "timestamp INTEGER,"
                      "type TEXT,"
                      "content TEXT,"
                      "tags TEXT);";
    
    if (!execute(sql)) return false;
    
    build_index();
    return true;
}

bool MemoryStore::execute(const std::string& sql) {
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db_, sql.c_str(), 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}

bool MemoryStore::store(const std::string& type, const std::string& content, const std::string& tags) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (!db_) return false;

    long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO memories (timestamp, type, content, tags) VALUES (?, ?, ?, ?);";
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, 0) != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, timestamp);
    sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, tags.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    
    // Index the new memory
    int id = static_cast<int>(sqlite3_last_insert_rowid(db_));
    index_memory(id, content);
    
    // Invalidate relevant cache - simplistic approach: clear specific keys? 
    // For now, we accept cache is slightly stale for 60s or we rely on TTL.
    
    return true;
}

// Global or static instance to share connection across calls
static RedisClient redis_cache_("redis");

std::vector<Memory> MemoryStore::query(const std::string& keyword) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    // Cache Key
    std::string cache_key = "query:" + keyword;
    
    std::vector<Memory> results;

    // 1. Check Redis
    auto cached = redis_cache_.get(cache_key);
    if (cached) {
        // Deserialize CSV: ID|Timestamp|Type|Content|Tags\n
        std::stringstream ss(cached.value());
        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            std::stringstream line_ss(line);
            std::string segment;
            std::vector<std::string> parts;
            while(std::getline(line_ss, segment, '|')) {
                parts.push_back(segment);
            }
            if (parts.size() >= 5) {
                Memory m;
                m.id = std::stoi(parts[0]);
                m.timestamp = std::stoll(parts[1]);
                m.type = parts[2];
                m.content = parts[3];
                m.tags = parts[4];
                results.push_back(m);
            }
        }
        return results;
    }
    
    
    if (!db_) return results;


    // Normalize keyword
    std::string term = keyword;
    std::transform(term.begin(), term.end(), term.begin(), ::tolower);
    
    if (inverted_index_.find(term) == inverted_index_.end()) {
        return results; // No matches found (O(1))
    }

    const auto& ids = inverted_index_[term];
    if (ids.empty()) return results;

    // Create SQL for specific IDs
    // Get last 20 IDs (most recent)
    std::string id_list = "";
    int count = 0;
    // Iterate backwards
    for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
        if (!id_list.empty()) id_list += ",";
        id_list += std::to_string(*it);
        if (++count >= 20) break;
    }

    std::string sql_str = "SELECT id, timestamp, type, content, tags FROM memories WHERE id IN (" + id_list + ") ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql_str.c_str(), -1, &stmt, 0) != SQLITE_OK) return results;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Memory m;
        m.id = sqlite3_column_int(stmt, 0);
        m.timestamp = sqlite3_column_int64(stmt, 1);
        m.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        m.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        m.tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        results.push_back(m);
    }
    sqlite3_finalize(stmt);
    
    // Store in Redis (Serialize)
    if (!results.empty()) {
        std::string serialized = "";
        for (const auto& m : results) {
            serialized += std::to_string(m.id) + "|" + 
                          std::to_string(m.timestamp) + "|" + 
                          m.type + "|" + 
                          m.content + "|" + 
                          m.tags + "\n";
        }
        redis_cache_.set(cache_key, serialized, 60); // TTL 60s
    }
    
    return results;
}

void MemoryStore::build_index() {
    inverted_index_.clear();
    std::string sql = "SELECT id, content FROM memories ORDER BY id ASC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, 0) != SQLITE_OK) return;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        index_memory(id, content);
    }
    sqlite3_finalize(stmt);
    std::cout << "[MemoryStore] Index built. Tokens: " << inverted_index_.size() << std::endl;
}

void MemoryStore::index_memory(int id, const std::string& content) {
    std::string local_content = content;
    // Simple tokenizer: split by non-alpha
    for (size_t i = 0; i < local_content.size(); ++i) {
        if (std::isalpha(local_content[i])) {
             local_content[i] = static_cast<char>(std::tolower(local_content[i]));
        } else {
             local_content[i] = ' ';
        }
    }
    
    std::stringstream ss(local_content);
    std::string token;
    while (ss >> token) {
        if (token.length() < 3) continue; // Skip small words
        inverted_index_[token].push_back(id);
    }
}

std::vector<Memory> MemoryStore::get_recent(int limit) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::vector<Memory> results;
    if (!db_) return results;

    std::string sql_str = "SELECT id, timestamp, type, content, tags FROM memories ORDER BY timestamp DESC LIMIT ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql_str.c_str(), -1, &stmt, 0) != SQLITE_OK) return results;
    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Memory m;
        m.id = sqlite3_column_int(stmt, 0);
        m.timestamp = sqlite3_column_int64(stmt, 1);
        m.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        m.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        m.tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        results.push_back(m);
    }
    sqlite3_finalize(stmt);
    return results;
}

long long MemoryStore::get_memory_count() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (!db_) return 0;
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM memories;", -1, &stmt, 0) != SQLITE_OK) return 0;
    
    long long count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}
