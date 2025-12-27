#include "postgres_storage.hpp"
#include <iostream>
#include <stdexcept>

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

    // Ensure table exists
    execute_non_query(
        "CREATE TABLE IF NOT EXISTS brain_kv_store ("
        "key TEXT PRIMARY KEY, "
        "value TEXT)" 
    ); // Assuming pgvector extension is available, simplistic fallback if not

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
void PostgresStorage::store_embedding(const std::string& key, const std::vector<double>& embedding) {
    // Implementation deferred until pgvector is confirmed
}

std::vector<std::string> PostgresStorage::search_similar(const std::vector<double>& embedding, int limit) {
    // Implementation deferred
    return {};
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
