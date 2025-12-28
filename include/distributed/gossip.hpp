#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <chrono>

namespace dnn::distributed {

// Gossip Protocol for epidemic-style information dissemination
class GossipProtocol {
public:
    struct Message {
        std::string id;
        std::string content;
        std::string origin_node;
        int64_t timestamp;
        int hop_count;
        
        Message() : timestamp(0), hop_count(0) {}
        
        Message(const std::string& msg_id, const std::string& data, 
               const std::string& origin, int hops = 0)
            : id(msg_id), content(data), origin_node(origin), hop_count(hops) {
            timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        }
    };
    
    struct Config {
        int fanout;              // Number of peers to gossip to
        int max_hops;            // Maximum propagation hops
        int message_ttl_ms;      // Message time-to-live
        float gossip_probability; // Probability of gossiping
        
        Config() : fanout(3), max_hops(10), message_ttl_ms(60000), 
                  gossip_probability(0.8f) {}
    };
    
    GossipProtocol(const std::string& node_id, const Config& config = Config())
        : node_id_(node_id), config_(config), gen_(std::random_device{}()) {}
    
    // Add a peer to gossip with
    void add_peer(const std::string& peer_id) {
        peers_.insert(peer_id);
    }
    
    // Remove a peer
    void remove_peer(const std::string& peer_id) {
        peers_.erase(peer_id);
    }
    
    // Initiate a new message
    std::string broadcast(const std::string& content) {
        std::string msg_id = generate_message_id();
        Message msg(msg_id, content, node_id_, 0);
        
        seen_messages_[msg_id] = msg;
        return msg_id;
    }
    
    // Receive a message from a peer
    bool receive(const Message& msg) {
        // Check if already seen
        if (seen_messages_.count(msg.id)) {
            return false;
        }
        
        // Check TTL
        int64_t age = std::chrono::system_clock::now().time_since_epoch().count() - msg.timestamp;
        if (age > config_.message_ttl_ms * 1000000LL) {
            return false;
        }
        
        // Check hop count
        if (msg.hop_count >= config_.max_hops) {
            return false;
        }
        
        // Store message
        seen_messages_[msg.id] = msg;
        pending_gossip_.push_back(msg);
        
        return true;
    }
    
    // Get peers to gossip to
    std::vector<std::string> select_gossip_targets() {
        std::vector<std::string> targets;
        
        if (peers_.empty()) {
            return targets;
        }
        
        // Random selection with fanout
        std::vector<std::string> peer_list(peers_.begin(), peers_.end());
        
        std::uniform_int_distribution<size_t> dist(0, peer_list.size() - 1);
        std::uniform_real_distribution<float> prob_dist(0.0f, 1.0f);
        
        int selected = 0;
        while (selected < config_.fanout && selected < static_cast<int>(peer_list.size())) {
            if (prob_dist(gen_) < config_.gossip_probability) {
                size_t idx = dist(gen_);
                targets.push_back(peer_list[idx]);
                peer_list.erase(peer_list.begin() + idx);
            }
            selected++;
        }
        
        return targets;
    }
    
    // Get pending messages to gossip
    std::vector<Message> get_pending_messages() {
        std::vector<Message> messages;
        
        for (auto& msg : pending_gossip_) {
            msg.hop_count++;
            messages.push_back(msg);
        }
        
        pending_gossip_.clear();
        return messages;
    }
    
    // Get all received messages
    std::vector<Message> get_all_messages() const {
        std::vector<Message> messages;
        messages.reserve(seen_messages_.size());
        
        for (const auto& [id, msg] : seen_messages_) {
            messages.push_back(msg);
        }
        
        return messages;
    }
    
    // Get message by ID
    Message* get_message(const std::string& msg_id) {
        auto it = seen_messages_.find(msg_id);
        if (it != seen_messages_.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    // Clean up old messages
    void cleanup_old_messages() {
        int64_t now = std::chrono::system_clock::now().time_since_epoch().count();
        
        std::vector<std::string> to_remove;
        for (const auto& [id, msg] : seen_messages_) {
            int64_t age = now - msg.timestamp;
            if (age > config_.message_ttl_ms * 1000000LL) {
                to_remove.push_back(id);
            }
        }
        
        for (const auto& id : to_remove) {
            seen_messages_.erase(id);
        }
    }
    
    // Get statistics
    struct Stats {
        int total_messages;
        int unique_origins;
        int pending_gossip;
        int peer_count;
    };
    
    Stats get_stats() const {
        Stats stats{};
        stats.total_messages = seen_messages_.size();
        stats.pending_gossip = pending_gossip_.size();
        stats.peer_count = peers_.size();
        
        std::unordered_set<std::string> origins;
        for (const auto& [id, msg] : seen_messages_) {
            origins.insert(msg.origin_node);
        }
        stats.unique_origins = origins.size();
        
        return stats;
    }
    
private:
    std::string node_id_;
    Config config_;
    std::unordered_set<std::string> peers_;
    std::unordered_map<std::string, Message> seen_messages_;
    std::vector<Message> pending_gossip_;
    std::mt19937 gen_;
    int message_counter_ = 0;
    
    std::string generate_message_id() {
        return node_id_ + "_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) 
               + "_" + std::to_string(message_counter_++);
    }
};

} // namespace dnn::distributed
