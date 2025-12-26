#include "postgres_client.hpp"
#include <iostream>

PostgresClient::PostgresClient(const std::string& conn_str) 
    : connection_string(conn_str), conn(nullptr), connected(false) {}

PostgresClient::~PostgresClient() {
    disconnect();
}

bool PostgresClient::connect() {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (connected) return true;

    conn = PQconnectdb(connection_string.c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "PostgreSQL connection failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        conn = nullptr;
        return false;
    }

    connected = true;
    std::cout << "[PostgresClient] Connected successfully." << std::endl;
    return true;
}

void PostgresClient::disconnect() {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
    connected = false;
}

bool PostgresClient::execute(const std::string& sql) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!connected) return false;

    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "PostgreSQL execution failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

std::vector<PostgresRow> PostgresClient::query(const std::string& sql) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<PostgresRow> results;
    if (!connected) return results;

    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "PostgreSQL query failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return results;
    }

    int rows = PQntuples(res);
    int cols = PQnfields(res);

    for (int i = 0; i < rows; i++) {
        PostgresRow row;
        for (int j = 0; j < cols; j++) {
            row.columns.push_back(PQgetvalue(res, i, j));
        }
        results.push_back(row);
    }

    PQclear(res);
    return results;
}

int PostgresClient::store_memory(long long timestamp, const std::string& type, const std::string& content, const std::string& tags) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!connected) return -1;

    const char* paramValues[4];
    std::string ts_str = std::to_string(timestamp);
    paramValues[0] = ts_str.c_str();
    paramValues[1] = type.c_str();
    paramValues[2] = content.c_str();
    paramValues[3] = tags.c_str();

    PGresult* res = PQexecParams(conn,
        "INSERT INTO memories (timestamp, type, content, tags) VALUES ($1, $2, $3, $4) RETURNING id",
        4,       /* four parameters */
        NULL,    /* let the backend deduce param types */
        paramValues,
        NULL,    /* text format */
        NULL,    /* text format */
        0);      /* text results */

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "PostgreSQL insert failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return -1;
    }

    int id = std::stoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return id;
}
