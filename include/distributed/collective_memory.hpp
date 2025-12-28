#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <chrono>
#include <algorithm>

namespace dnn::distributed {

// Collective memory system for multi-agent knowledge sharing
class CollectiveMemory {
public:
    struct MemoryEntry {
        std::string key;
        std::string value;
        std::string contributor_id;
        float confidence;
        int64_t timestamp;
        int version;
        
        MemoryEntry() : confidence(1.0f), timestamp(0), version(1) {}
        
        MemoryEntry(const std::string& k, const std::string& v, 
                   const std::string& contrib, float conf = 1.0f)
            : key(k), value(v), contributor_id(contrib), confidence(conf), version(1) {
            timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        }
    };
    
    enum class MergeStrategy {
        LATEST_WINS,          // Most recent update wins
        HIGHEST_CONFIDENCE,   // Highest confidence value wins
        VOTE,                 // Majority consensus
        APPEND                // Keep all versions
    };
    
    CollectiveMemory(MergeStrategy strategy = MergeStrategy::LATEST_WINS)
        : merge_strategy_(strategy) {}
    
    // Store a memory with contributor ID
    void store(const std::string& key, const std::string& value, 
              const std::string& contributor_id, float confidence = 1.0f) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = memory_.find(key);
        if (it == memory_.end()) {
            // New entry
            memory_[key] = MemoryEntry(key, value, contributor_id, confidence);
        } else {
            // Merge with existing
            merge_entry(it->second, value, contributor_id, confidence);
        }
    }
    
    // Retrieve memory by key
    std::string retrieve(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = memory_.find(key);
        if (it != memory_.end()) {
            return it->second.value;
        }
        return "";
    }
    
    // Get entry with metadata
    MemoryEntry get_entry(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = memory_.find(key);
        if (it != memory_.end()) {
            return it->second;
        }
        return MemoryEntry();
    }
    
    // Query entries by contributor
    std::vector<MemoryEntry> query_by_contributor(const std::string& contributor_id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<MemoryEntry> results;
        for (const auto& [key, entry] : memory_) {
            if (entry.contributor_id == contributor_id) {
                results.push_back(entry);
            }
        }
        return results;
    }
    
    // Query entries with minimum confidence
    std::vector<MemoryEntry> query_by_confidence(float min_confidence) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<MemoryEntry> results;
        for (const auto& [key, entry] : memory_) {
            if (entry.confidence >= min_confidence) {
                results.push_back(entry);
            }
        }
        return results;
    }
    
    // Get all keys
    std::vector<std::string> get_all_keys() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<std::string> keys;
        keys.reserve(memory_.size());
        for (const auto& [key, _] : memory_) {
            keys.push_back(key);
        }
        return keys;
    }
    
    // Merge memories from another agent
    void merge_from(const CollectiveMemory& other) {
        auto other_keys = other.get_all_keys();
        for (const auto& key : other_keys) {
            auto entry = other.get_entry(key);
            store(entry.key, entry.value, entry.contributor_id, entry.confidence);
        }
    }
    
    // Get statistics
    struct Stats {
        size_t total_entries;
        size_t unique_contributors;
        float avg_confidence;
        int64_t oldest_timestamp;
        int64_t newest_timestamp;
    };
    
    Stats get_stats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        Stats stats{};
        stats.total_entries = memory_.size();
        
        if (memory_.empty()) return stats;
        
        std::unordered_set<std::string> contributors;
        float total_confidence = 0.0f;
        int64_t oldest = std::numeric_limits<int64_t>::max();
        int64_t newest = std::numeric_limits<int64_t>::min();
        
        for (const auto& [key, entry] : memory_) {
            contributors.insert(entry.contributor_id);
            total_confidence += entry.confidence;
            oldest = std::min(oldest, entry.timestamp);
            newest = std::max(newest, entry.timestamp);
        }
        
        stats.unique_contributors = contributors.size();
        stats.avg_confidence = total_confidence / memory_.size();
        stats.oldest_timestamp = oldest;
        stats.newest_timestamp = newest;
        
        return stats;
    }
    
    // Clear all memories
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        memory_.clear();
    }
    
    // Remove entry by key
    bool remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        return memory_.erase(key) > 0;
    }
    
    // Export to vector for serialization
    std::vector<MemoryEntry> export_all() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<MemoryEntry> entries;
        entries.reserve(memory_.size());
        for (const auto& [key, entry] : memory_) {
            entries.push_back(entry);
        }
        return entries;
    }
    
    // Import from vector
    void import_all(const std::vector<MemoryEntry>& entries) {
        for (const auto& entry : entries) {
            store(entry.key, entry.value, entry.contributor_id, entry.confidence);
        }
    }
    
private:
    MergeStrategy merge_strategy_;
    std::unordered_map<std::string, MemoryEntry> memory_;
    mutable std::mutex mutex_;
    
    void merge_entry(MemoryEntry& existing, const std::string& new_value,
                    const std::string& contributor_id, float confidence) {
        switch (merge_strategy_) {
            case MergeStrategy::LATEST_WINS: {
                // Always use the new value
                existing.value = new_value;
                existing.contributor_id = contributor_id;
                existing.confidence = confidence;
                existing.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                existing.version++;
                break;
            }
            
            case MergeStrategy::HIGHEST_CONFIDENCE: {
                // Use value with highest confidence
                if (confidence > existing.confidence) {
                    existing.value = new_value;
                    existing.contributor_id = contributor_id;
                    existing.confidence = confidence;
                    existing.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                    existing.version++;
                }
                break;
            }
            
            case MergeStrategy::VOTE: {
                // Simple voting: if same value, increase confidence
                if (existing.value == new_value) {
                    existing.confidence = std::min(1.0f, existing.confidence + 0.1f);
                } else {
                    // Different value with higher confidence wins
                    if (confidence > existing.confidence) {
                        existing.value = new_value;
                        existing.contributor_id = contributor_id;
                        existing.confidence = confidence;
                    }
                }
                existing.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                existing.version++;
                break;
            }
            
            case MergeStrategy::APPEND: {
                // Append new value to existing (comma-separated)
                if (!existing.value.empty() && existing.value != new_value) {
                    existing.value += "," + new_value;
                } else if (existing.value.empty()) {
                    existing.value = new_value;
                }
                existing.contributor_id += "," + contributor_id;
                existing.confidence = (existing.confidence + confidence) / 2.0f;
                existing.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                existing.version++;
                break;
            }
        }
    }
};

} // namespace dnn::distributed
