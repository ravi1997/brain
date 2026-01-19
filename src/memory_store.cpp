#ifdef USE_POSTGRES
#include "memory_store.hpp"
#include "redis_client.hpp"
#include <chrono>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

MemoryStore::MemoryStore(const std::string& conn_str) : conn_str_(conn_str) {
    pg_client = std::make_unique<PostgresClient>(conn_str);
    kv_store = std::make_unique<PostgresStorage>(conn_str);
}

MemoryStore::~MemoryStore() {
    // pg_client disconnects on destruction
}

bool MemoryStore::init() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (!pg_client->connect()) return false;
    if (kv_store && !kv_store->connect()) return false;

    // Create table if not exists (PostgreSQL syntax)
    const char* sql = "CREATE TABLE IF NOT EXISTS memories ("
                      "id SERIAL PRIMARY KEY,"
                      "timestamp BIGINT,"
                      "type TEXT,"
                      "content TEXT,"
                      "tags TEXT,"
                      "acl TEXT DEFAULT 'PUBLIC',"
                      "strength DOUBLE PRECISION DEFAULT 1.0,"
                      "last_recall BIGINT DEFAULT 0);";
    
    if (!pg_client->execute(sql)) return false;
    
    build_index();
    return true;
}

bool MemoryStore::store(const std::string& type, const std::string& content, const std::string& tags, const std::string& acl) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (!pg_client->is_connected()) return false;

    long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // WARNING: PostgresClient::store_memory stub needs update to handle ACL/Strength columns
    // For now we just call it and assume DB schema migration is handled manually or by SQL above
    int id = pg_client->store_memory(timestamp, type, content, tags);
    if (id == -1) return false;
    
    // Feature 6: Update ACL (Manual SQL until client updated)
    std::string update_sql = "UPDATE memories SET acl = '" + acl + "', strength = 1.0, last_recall = " + std::to_string(timestamp) + " WHERE id = " + std::to_string(id) + ";";
    pg_client->execute(update_sql.c_str());
    
    index_memory(id, content);
    
    return true;
}

// Global or static instance to share connection across calls
static RedisClient redis_cache_("redis");

std::vector<Memory> MemoryStore::query(const std::string& keyword, const std::string& user_acl) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    std::string cache_key = "query:" + keyword;
    std::vector<Memory> results;

    // 1. Check Redis
    auto cached = redis_cache_.get(cache_key);
    if (cached) {
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
    
    if (!pg_client->is_connected()) return results;

    std::string term = keyword;
    std::transform(term.begin(), term.end(), term.begin(), ::tolower);
    
    if (inverted_index_.find(term) == inverted_index_.end()) {
        return results;
    }

    const auto& ids = inverted_index_[term];
    if (ids.empty()) return results;

    std::string id_list = "";
    int count = 0;
    for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
        if (!id_list.empty()) id_list += ",";
        id_list += std::to_string(*it);
        if (++count >= 20) break;
    }

    std::string sql_str = "SELECT id, timestamp, type, content, tags, acl FROM memories WHERE id IN (" + id_list + ") ORDER BY timestamp DESC;";
    auto rows = pg_client->query(sql_str);

    for (const auto& row : rows) {
        // ACL Check
        std::string acl = row.columns.size() > 5 ? row.columns[5] : "PUBLIC";
        if (acl != "PUBLIC" && acl != user_acl) continue; // Basic ACL simulation

        Memory m;
        m.id = std::stoi(row.columns[0]);
        m.timestamp = std::stoll(row.columns[1]);
        m.type = row.columns[2];
        m.content = row.columns[3];
        m.tags = row.columns[4];
        m.acl_label = acl;
        results.push_back(m);
    }
    
    // Store in Redis
    if (!results.empty()) {
        std::string serialized = "";
        for (const auto& m : results) {
            serialized += std::to_string(m.id) + "|" + 
                          std::to_string(m.timestamp) + "|" + 
                          m.type + "|" + 
                          m.content + "|" + 
                          m.tags + "\n";
        }
        redis_cache_.set(cache_key, serialized, 60);
    }
    
    return results;
}

void MemoryStore::build_index() {
    inverted_index_.clear();
    if (!pg_client->is_connected()) return;

    auto rows = pg_client->query("SELECT id, content FROM memories ORDER BY id ASC;");
    for (const auto& row : rows) {
        int id = std::stoi(row.columns[0]);
        index_memory(id, row.columns[1]);
    }
    std::cout << "[MemoryStore] Index built from PostgreSQL. Tokens: " << inverted_index_.size() << std::endl;
}

void MemoryStore::index_memory(int id, const std::string& content) {
    std::string local_content = content;
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
        if (token.length() < 3) continue;
        inverted_index_[token].push_back(id);
    }
}

std::vector<Memory> MemoryStore::get_recent(int limit) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::vector<Memory> results;
    if (!pg_client->is_connected()) return results;

    std::string sql_str = "SELECT id, timestamp, type, content, tags FROM memories ORDER BY timestamp DESC LIMIT " + std::to_string(limit) + ";";
    auto rows = pg_client->query(sql_str);

    for (const auto& row : rows) {
        Memory m;
        m.id = std::stoi(row.columns[0]);
        m.timestamp = std::stoll(row.columns[1]);
        m.type = row.columns[2];
        m.content = row.columns[3];
        m.tags = row.columns[4];
        results.push_back(m);
    }
    return results;
}

long long MemoryStore::get_memory_count() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (!pg_client->is_connected()) return 0;
    
    auto res = pg_client->query("SELECT COUNT(*) FROM memories;");
    if (res.empty()) return 0;
    return std::stoll(res[0].columns[0]);
}

std::string MemoryStore::get_graph_json(int max_nodes) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    
    // 1. Pick top N tokens by frequency
    std::vector<std::pair<std::string, int>> frequencies;
    for (const auto& [token, ids] : inverted_index_) {
        frequencies.push_back({token, (int)ids.size()});
    }
    
    std::sort(frequencies.begin(), frequencies.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    int actual_nodes = std::min((int)frequencies.size(), max_nodes);
    std::vector<std::string> top_tokens;
    std::unordered_map<std::string, int> token_to_freq;
    for (int i = 0; i < actual_nodes; i++) {
        top_tokens.push_back(frequencies[i].first);
        token_to_freq[frequencies[i].first] = frequencies[i].second;
    }
    
    // 2. Build adjacency (shared memory IDs)
    std::stringstream ss;
    ss << "{\"type\": \"graph\", \"nodes\": [";
    for (int i = 0; i < actual_nodes; i++) {
        ss << "{\"id\": \"" << top_tokens[i] << "\", \"val\": " << token_to_freq[top_tokens[i]] << "}";
        if (i < actual_nodes - 1) ss << ",";
    }
    ss << "], \"links\": [";
    
    std::vector<std::string> links;
    #pragma omp parallel
    {
        std::vector<std::string> local_links;
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < actual_nodes; i++) {
            for (int j = i + 1; j < actual_nodes; j++) {
                const auto& ids1 = inverted_index_[top_tokens[i]];
                const auto& ids2 = inverted_index_[top_tokens[j]];
                
                int weight = 0;
                size_t p1 = 0, p2 = 0;
                while(p1 < ids1.size() && p2 < ids2.size()) {
                    if (ids1[p1] == ids2[p2]) {
                        weight++; p1++; p2++;
                    } else if (ids1[p1] < ids2[p2]) {
                        p1++;
                    } else {
                        p2++;
                    }
                }
                
                if (weight > 0) {
                    std::string link = "{\"source\": \"" + top_tokens[i] + "\", \"target\": \"" + top_tokens[j] + "\", \"weight\": " + std::to_string(weight) + "}";
                    local_links.push_back(link);
                }
            }
        }
        #pragma omp critical
        links.insert(links.end(), local_links.begin(), local_links.end());
    }

    for (size_t i = 0; i < links.size(); i++) {
        ss << links[i];
        if (i < links.size() - 1) ss << ",";
    }
    ss << "]}";
    return ss.str();
}
void MemoryStore::clear() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (pg_client->is_connected()) {
        pg_client->execute("TRUNCATE memories RESTART IDENTITY;");
    }
    inverted_index_.clear();
    // Cache remains, but since we use TRUNCATE identity, new IDs will match old ones
    // and might cause incorrect cache hits. We should probably flush Redis too.
}

void MemoryStore::store_embedding(const std::string& key, const std::vector<double>& embedding) {
    if (kv_store) kv_store->store_embedding(key, embedding);
}

std::vector<double> MemoryStore::retrieve_embedding(const std::string& key) {
    if (kv_store) return kv_store->retrieve_embedding(key);
    return {};
}

std::vector<std::string> MemoryStore::search_similar(const std::vector<double>& embedding, int limit) {
    if (kv_store) return kv_store->search_similar(embedding, limit);
    return {};
}
#endif
