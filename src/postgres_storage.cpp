#ifdef USE_POSTGRES
#include "postgres_storage.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>

PostgresStorage::PostgresStorage(const std::string& conn_str) 
    : connection_string(conn_str), conn(nullptr) {}

PostgresStorage::~PostgresStorage() {
    disconnect();
}

bool PostgresStorage::connect() {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (conn) {
        if (PQstatus(conn) == CONNECTION_OK) return true;
        PQfinish(conn);
    }
    
    conn = PQconnectdb(connection_string.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "[Postgres] Connection failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        conn = nullptr;
        return false;
    }

    // Enable pgvector extension
    execute_non_query("CREATE EXTENSION IF NOT EXISTS vector");

    // Ensure table exists
    execute_non_query(
        "CREATE TABLE IF NOT EXISTS brain_kv_store ("
        "key TEXT PRIMARY KEY, "
        "value TEXT, "
        "embedding vector(384))"
    );

    // Ensure embedding column exists (for migration)
    execute_non_query(
        "ALTER TABLE brain_kv_store ADD COLUMN IF NOT EXISTS embedding vector(384)"
    );

    // Add HNSW index for high-scale vector search
    execute_non_query(
        "CREATE INDEX IF NOT EXISTS brain_vector_idx ON brain_kv_store "
        "USING hnsw (embedding vector_cosine_ops) WITH (m = 16, ef_construction = 64)"
    );

    return true;
}

void PostgresStorage::disconnect() {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
}

void PostgresStorage::store_memory(const std::string& key, const std::string& value) {
    if (!check_status()) return;
    std::lock_guard<std::mutex> lock(db_mutex);

    const char* paramValues[2] = { key.c_str(), value.c_str() };
    PGresult* res = PQexecParams(conn,
        "INSERT INTO brain_kv_store (key, value) VALUES ($1, $2) "
        "ON CONFLICT (key) DO UPDATE SET value = $2",
        2, nullptr, paramValues, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "[Postgres] Store failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQclear(res);
}

void PostgresStorage::store_memories_bulk(const std::map<std::string, std::string>& memories) {
    if (!check_status() || memories.empty()) return;
    std::lock_guard<std::mutex> lock(db_mutex);

    execute_non_query("BEGIN");

    for (const auto& [key, value] : memories) {
        const char* paramValues[2] = { key.c_str(), value.c_str() };
        PGresult* res = PQexecParams(conn,
            "INSERT INTO brain_kv_store (key, value) VALUES ($1, $2) "
            "ON CONFLICT (key) DO UPDATE SET value = $2",
            2, nullptr, paramValues, nullptr, nullptr, 0);
        
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "[Postgres] Bulk Store failed for key " << key << ": " << PQerrorMessage(conn) << std::endl;
        }
        PQclear(res);
    }

    execute_non_query("COMMIT");
}

std::string PostgresStorage::retrieve_memory(const std::string& key) {
    if (!check_status()) return "";
    std::lock_guard<std::mutex> lock(db_mutex);

    const char* paramValues[1] = { key.c_str() };
    PGresult* res = PQexecParams(conn,
        "SELECT value FROM brain_kv_store WHERE key = $1",
        1, nullptr, paramValues, nullptr, nullptr, 0);

    std::string value;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        value = PQgetvalue(res, 0, 0);
    }
    PQclear(res);
    return value;
}

// Placeholder for embedding storage (requires pgvector setup)
// Helper to serialize vector to string for pgvector
static std::string vector_to_string(const std::vector<double>& vec) {
    std::string s = "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        s += std::to_string(vec[i]);
        if (i < vec.size() - 1) s += ",";
    }
    s += "]";
    return s;
}

static std::vector<double> string_to_vector(const std::string& s) {
    std::vector<double> vec;
    if (s.empty() || s.front() != '[') return vec;
    
    std::string content = s.substr(1, s.length() - 2); // Remove []
    std::stringstream ss(content);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        try {
            vec.push_back(std::stod(item));
        } catch (...) {}
    }
    return vec;
}

void PostgresStorage::store_embedding(const std::string& key, const std::vector<double>& embedding) {
    if (!check_status()) return;
    std::lock_guard<std::mutex> lock(db_mutex);

    std::string embed_str = vector_to_string(embedding);
    const char* paramValues[2] = { key.c_str(), embed_str.c_str() };

    // Upsert: Insert if new (with empty value), or update embedding if exists
    PGresult* res = PQexecParams(conn,
        "INSERT INTO brain_kv_store (key, value, embedding) VALUES ($1, '', $2) "
        "ON CONFLICT (key) DO UPDATE SET embedding = $2",
        2, nullptr, paramValues, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "[Postgres] Store Embedding failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQclear(res);
}

std::vector<double> PostgresStorage::retrieve_embedding(const std::string& key) {
    if (!check_status()) return {};
    std::lock_guard<std::mutex> lock(db_mutex);

    const char* paramValues[1] = { key.c_str() };
    PGresult* res = PQexecParams(conn,
        "SELECT embedding FROM brain_kv_store WHERE key = $1",
        1, nullptr, paramValues, nullptr, nullptr, 0);

    std::vector<double> vec;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        char* val = PQgetvalue(res, 0, 0);
        if (val) vec = string_to_vector(val);
    }
    PQclear(res);
    return vec;
}

std::vector<std::string> PostgresStorage::search_similar(const std::vector<double>& embedding, int limit) {
    if (!check_status()) return {};
    std::lock_guard<std::mutex> lock(db_mutex);

    std::string embed_str = vector_to_string(embedding);
    std::string limit_str = std::to_string(limit);
    const char* paramValues[2] = { embed_str.c_str(), limit_str.c_str() };

    // Cosine distance sort
    PGresult* res = PQexecParams(conn,
        "SELECT key FROM brain_kv_store ORDER BY embedding <=> $1 LIMIT $2",
        2, nullptr, paramValues, nullptr, nullptr, 0);

    std::vector<std::string> results;
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; ++i) {
            results.push_back(PQgetvalue(res, i, 0));
        }
    } else {
        std::cerr << "[Postgres] Search failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQclear(res);
    return results;
}

bool PostgresStorage::begin_transaction() {
    if (!check_status()) return false;
    execute_non_query("BEGIN");
    return true;
}

bool PostgresStorage::commit() {
    if (!check_status()) return false;
    execute_non_query("COMMIT");
    return true;
}

bool PostgresStorage::rollback() {
    if (!check_status()) return false;
    execute_non_query("ROLLBACK");
    return true;
}

bool PostgresStorage::check_status() {
    if (!conn || PQstatus(conn) != CONNECTION_OK) {
        return connect();
    }
    return true;
}

void PostgresStorage::execute_non_query(const std::string& sql) {
    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "[Postgres] Query failed: " << PQerrorMessage(conn) << std::endl;
    }
    PQclear(res);
}
#endif
