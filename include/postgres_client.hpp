#pragma once
#include <string>
#include <vector>
#ifdef USE_POSTGRES
#include <libpq-fe.h>
#endif
#include <mutex>

struct PostgresRow {
    std::vector<std::string> columns;
};

class PostgresClient {
public:
    PostgresClient(const std::string& conn_str);
    ~PostgresClient();
    
    bool connect();
    void disconnect();
    bool is_connected() const { return connected; }
    
    bool execute(const std::string& sql);
    std::vector<PostgresRow> query(const std::string& sql);
    
    // Helper for parameterized inserts (returns ID or -1)
    int store_memory(long long timestamp, const std::string& type, const std::string& content, const std::string& tags);

private:
    std::string connection_string;
    PGconn* conn;
    bool connected;
    mutable std::mutex db_mutex;
};
