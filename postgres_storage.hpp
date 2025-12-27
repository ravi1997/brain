#pragma once

#include "db_interface.hpp"
#include <libpq-fe.h>
#include <mutex>

class PostgresStorage : public DatabaseInterface {
public:
    PostgresStorage(const std::string& conn_str);
    ~PostgresStorage() override;

    bool connect() override;
    void disconnect() override;

    void store_memory(const std::string& key, const std::string& value) override;
    std::string retrieve_memory(const std::string& key) override;

    void store_embedding(const std::string& key, const std::vector<double>& embedding) override;
    std::vector<std::string> search_similar(const std::vector<double>& embedding, int limit) override;
    
    // Transaction support
    bool begin_transaction() override;
    bool commit() override;
    bool rollback() override;

private:
    std::string connection_string;
    PGconn* conn;
    std::mutex db_mutex;

    bool check_status();
    void execute_non_query(const std::string& sql);
};
