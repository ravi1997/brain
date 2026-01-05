#pragma once
#include <string>
#include <vector>
#include <map>

// Abstract Base Class for Long-Term Memory Storage
// Prepares for migration from Files/Redis to PostgreSQL
class DatabaseInterface {
public:
    virtual ~DatabaseInterface() = default;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;

    // Core Memory Operations
    virtual void store_memory(const std::string& key, const std::string& value) = 0;
    virtual void store_memories_bulk(const std::map<std::string, std::string>& memories) = 0;
    virtual std::string retrieve_memory(const std::string& key) = 0;
    
    // For vector search support (future)
    virtual void store_embedding(const std::string& key, const std::vector<double>& embedding) = 0;
    virtual std::vector<double> retrieve_embedding(const std::string& key) = 0;
    virtual std::vector<std::string> search_similar(const std::vector<double>& embedding, int limit) = 0;

    // Transaction support
    virtual bool begin_transaction() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;
};
