#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <mutex>

namespace dnn::distributed {

// Peer-to-Peer Knowledge Sharing
class P2PKnowledge {
public:
    struct KnowledgeItem {
        std::string key;
        std::string value;
        std::string owner_peer;
        float confidence;
        int64_t version;
        std::vector<std::string> endorsers;  // Peers who validated this
        
        KnowledgeItem() : confidence(1.0f), version(1) {}
        
        KnowledgeItem(const std::string& k, const std::string& v, 
                     const std::string& owner, float conf = 1.0f)
            : key(k), value(v), owner_peer(owner), confidence(conf), version(1) {}
    };
    
    struct PeerInfo {
        std::string peer_id;
        int reputation_score;
        int64_t last_sync;
        int knowledge_count;
        
        PeerInfo(const std::string& id = "") 
            : peer_id(id), reputation_score(100), last_sync(0), knowledge_count(0) {}
    };
    
    P2PKnowledge(const std::string& my_peer_id)
        : peer_id_(my_peer_id) {}
    
    // Add a peer to the network
    void add_peer(const std::string& peer_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (peers_.find(peer_id) == peers_.end()) {
            peers_[peer_id] = PeerInfo(peer_id);
        }
    }
    
    // Share knowledge with the network
    void share(const std::string& key, const std::string& value, float confidence = 1.0f) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = knowledge_base_.find(key);
        if (it == knowledge_base_.end()) {
            knowledge_base_[key] = KnowledgeItem(key, value, peer_id_, confidence);
        } else {
            // Update if we have higher confidence
            if (confidence > it->second.confidence) {
                it->second.value = value;
                it->second.confidence = confidence;
                it->second.owner_peer = peer_id_;
                it->second.version++;
            }
        }
    }
    
    // Request knowledge from network
    KnowledgeItem request(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = knowledge_base_.find(key);
        if (it != knowledge_base_.end()) {
            return it->second;
        }
        
        return KnowledgeItem();
    }
    
    // Receive knowledge from another peer
    bool receive(const KnowledgeItem& item, const std::string& from_peer) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Update peer info
        if (peers_.count(from_peer)) {
            peers_[from_peer].knowledge_count++;
        }
        
        auto it = knowledge_base_.find(item.key);
        if (it == knowledge_base_.end()) {
            // New knowledge
            knowledge_base_[item.key] = item;
            return true;
        } else {
            // Merge by version and confidence
            if (item.version > it->second.version ||
                (item.version == it->second.version && item.confidence > it->second.confidence)) {
                it->second = item;
                return true;
            }
        }
        
        return false;
    }
    
    // Endorse a knowledge item
    void endorse(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = knowledge_base_.find(key);
        if (it != knowledge_base_.end()) {
            // Add endorsement if not already endorsed
            auto& endorsers = it->second.endorsers;
            if (std::find(endorsers.begin(), endorsers.end(), peer_id_) == endorsers.end()) {
                endorsers.push_back(peer_id_);
                
                // Increase confidence based on endorsements
                it->second.confidence = std::min(1.0f, 
                    it->second.confidence + 0.05f * static_cast<float>(endorsers.size()));
            }
        }
    }
    
    // Get all knowledge items
    std::vector<KnowledgeItem> get_all_knowledge() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<KnowledgeItem> items;
        items.reserve(knowledge_base_.size());
        
        for (const auto& [key, item] : knowledge_base_) {
            items.push_back(item);
        }
        
        return items;
    }
    
    // Query knowledge by minimum confidence
    std::vector<KnowledgeItem> query_by_confidence(float min_confidence) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<KnowledgeItem> results;
        for (const auto& [key, item] : knowledge_base_) {
            if (item.confidence >= min_confidence) {
                results.push_back(item);
            }
        }
        
        return results;
    }
    
    // Get knowledge from specific peer
    std::vector<KnowledgeItem> get_peer_knowledge(const std::string& peer_id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<KnowledgeItem> results;
        for (const auto& [key, item] : knowledge_base_) {
            if (item.owner_peer == peer_id) {
                results.push_back(item);
            }
        }
        
        return results;
    }
    
    // Update peer reputation
    void update_reputation(const std::string& peer_id, int delta) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (peers_.count(peer_id)) {
            peers_[peer_id].reputation_score += delta;
            peers_[peer_id].reputation_score = std::max(0, 
                std::min(100, peers_[peer_id].reputation_score));
        }
    }
    
    // Get trusted peers (reputation > threshold)
    std::vector<std::string> get_trusted_peers(int min_reputation = 70) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<std::string> trusted;
        for (const auto& [id, info] : peers_) {
            if (info.reputation_score >= min_reputation) {
                trusted.push_back(id);
            }
        }
        
        return trusted;
    }
    
    // Synchronize with a peer (exchange knowledge)
    std::vector<KnowledgeItem> prepare_sync_data(const std::string& peer_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Get items to share (owned by us or highly confident)
        std::vector<KnowledgeItem> sync_data;
        
        for (const auto& [key, item] : knowledge_base_) {
            if (item.owner_peer == peer_id_ || item.confidence > 0.8f) {
                sync_data.push_back(item);
            }
        }
        
        return sync_data;
    }
    
    // Get statistics
    struct Stats {
        int total_knowledge;
        int peer_count;
        float avg_confidence;
        int total_endorsements;
    };
    
    Stats get_stats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        Stats stats{};
        stats.total_knowledge = knowledge_base_.size();
        stats.peer_count = peers_.size();
        
        float total_conf = 0.0f;
        int total_endorse = 0;
        
        for (const auto& [key, item] : knowledge_base_) {
            total_conf += item.confidence;
            total_endorse += item.endorsers.size();
        }
        
        if (!knowledge_base_.empty()) {
            stats.avg_confidence = total_conf / knowledge_base_.size();
        }
        stats.total_endorsements = total_endorse;
        
        return stats;
    }
    
private:
    std::string peer_id_;
    std::unordered_map<std::string, KnowledgeItem> knowledge_base_;
    std::unordered_map<std::string, PeerInfo> peers_;
    mutable std::mutex mutex_;
};

} // namespace dnn::distributed
