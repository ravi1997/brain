#include "memory_store.hpp"
#include <chrono>
#include <iostream>

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
    
    return execute(sql);
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
    return true;
}

std::vector<Memory> MemoryStore::query(const std::string& keyword) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::vector<Memory> results;
    if (!db_) return results;

    std::string sql_str = "SELECT id, timestamp, type, content, tags FROM memories WHERE content LIKE ? ORDER BY timestamp DESC LIMIT 20;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql_str.c_str(), -1, &stmt, 0) != SQLITE_OK) return results;

    std::string pattern = "%" + keyword + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);

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
