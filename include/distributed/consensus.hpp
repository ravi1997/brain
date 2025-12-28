#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <random>
#include <functional>
#include <chrono>

namespace dnn::distributed {

// Raft Consensus Algorithm implementation
class ConsensusAlgorithm {
public:
    enum class NodeState {
        FOLLOWER,
        CANDIDATE,
        LEADER
    };
    
    struct LogEntry {
        int term;
        std::string command;
        int index;
        
        LogEntry(int t = 0, const std::string& cmd = "", int idx = 0)
            : term(t), command(cmd), index(idx) {}
    };
    
    struct Config {
        int election_timeout_min_ms;
        int election_timeout_max_ms;
        int heartbeat_interval_ms;
        
        Config() : election_timeout_min_ms(150), 
                  election_timeout_max_ms(300),
                  heartbeat_interval_ms(50) {}
    };
    
    ConsensusAlgorithm(const std::string& node_id, const std::vector<std::string>& peer_ids,
                      const Config& config = Config())
        : node_id_(node_id), peer_ids_(peer_ids), config_(config),
          state_(NodeState::FOLLOWER), current_term_(0), voted_for_(""),
          commit_index_(0), last_applied_(0), gen_(std::random_device{}()) {
        
        for (const auto& peer : peer_ids_) {
            next_index_[peer] = 1;
            match_index_[peer] = 0;
        }
    }
    
    // Get current state
    NodeState get_state() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_;
    }
    
    // Get current term
    int get_term() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_term_;
    }
    
    // Check if this node is the leader
    bool is_leader() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_ == NodeState::LEADER;
    }
    
    // Append a new command (leader only)
    bool append_command(const std::string& command) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (state_ != NodeState::LEADER) {
            return false;
        }
        
        LogEntry entry(current_term_, command, log_.size());
        log_.push_back(entry);
        return true;
    }
    
    // Start an election (become candidate)
    void start_election() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        state_ = NodeState::CANDIDATE;
        current_term_++;
        voted_for_ = node_id_;
        votes_received_ = 1;  // Vote for self
        
        last_election_time_ = get_current_time_ms();
    }
    
    // Receive vote request
    bool request_vote(int term, const std::string& candidate_id, 
                     int last_log_index, int last_log_term) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Reject if term is old
        if (term < current_term_) {
            return false;
        }
        
        // Update term if newer
        if (term > current_term_) {
            current_term_ = term;
            voted_for_ = "";
            state_ = NodeState::FOLLOWER;
        }
        
        // Check if already voted in this term
        if (!voted_for_.empty() && voted_for_ != candidate_id) {
            return false;
        }
        
        // Check log is at least as up-to-date
        int my_last_index = log_.empty() ? 0 : log_.back().index;
        int my_last_term = log_.empty() ? 0 : log_.back().term;
        
        bool log_ok = (last_log_term > my_last_term) ||
                     (last_log_term == my_last_term && last_log_index >= my_last_index);
        
        if (log_ok) {
            voted_for_ = candidate_id;
            last_election_time_ = get_current_time_ms();
            return true;
        }
        
        return false;
    }
    
    // Record a vote received
    void receive_vote(bool granted) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (state_ != NodeState::CANDIDATE) {
            return;
        }
        
        if (granted) {
            votes_received_++;
            
            // Check if won election (majority)
            int majority = (peer_ids_.size() + 1) / 2 + 1;
            if (votes_received_ >= majority) {
                become_leader();
            }
        }
    }
    
    // Append entries (from leader)
    bool append_entries(int term, const std::string& leader_id,
                       int prev_log_index, int prev_log_term,
                       const std::vector<LogEntry>& entries,
                       int leader_commit) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Reject if term is old
        if (term < current_term_) {
            return false;
        }
        
        // Update term and become follower
        if (term > current_term_) {
            current_term_ = term;
            voted_for_ = "";
        }
        state_ = NodeState::FOLLOWER;
        last_election_time_ = get_current_time_ms();
        
        // Check previous log match
        if (prev_log_index > 0) {
            if (prev_log_index > static_cast<int>(log_.size()) ||
                (prev_log_index <= static_cast<int>(log_.size()) && 
                 log_[prev_log_index - 1].term != prev_log_term)) {
                return false;
            }
        }
        
        // Append new entries
        for (size_t i = 0; i < entries.size(); i++) {
            int index = prev_log_index + i;
            if (index < static_cast<int>(log_.size())) {
                log_[index] = entries[i];
            } else {
                log_.push_back(entries[i]);
            }
        }
        
        // Update commit index
        if (leader_commit > commit_index_) {
            commit_index_ = std::min(leader_commit, static_cast<int>(log_.size()) - 1);
        }
        
        return true;
    }
    
    // Check if election timeout has occurred
    bool check_election_timeout() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (state_ == NodeState::LEADER) {
            return false;
        }
        
        int64_t elapsed = get_current_time_ms() - last_election_time_;
        int timeout = get_election_timeout_ms();
        
        return elapsed > timeout;
    }
    
    // Get log entries for replication
    std::vector<LogEntry> get_entries_for_peer(const std::string& peer_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        int next_idx = next_index_[peer_id];
        if (next_idx > static_cast<int>(log_.size())) {
            return {};
        }
        
        return std::vector<LogEntry>(log_.begin() + next_idx, log_.end());
    }
    
    // Get committed entries
    std::vector<LogEntry> get_committed_entries() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (commit_index_ == 0) {
            return {};
        }
        
        return std::vector<LogEntry>(log_.begin(), log_.begin() + commit_index_ + 1);
    }
    
private:
    std::string node_id_;
    std::vector<std::string> peer_ids_;
    Config config_;
    
    mutable std::mutex mutex_;
    NodeState state_;
    int current_term_;
    std::string voted_for_;
    int votes_received_;
    
    std::vector<LogEntry> log_;
    int commit_index_;
    int last_applied_;
    
    // Leader state
    std::unordered_map<std::string, int> next_index_;
    std::unordered_map<std::string, int> match_index_;
    
    int64_t last_election_time_;
    std::mt19937 gen_;
    
    void become_leader() {
        state_ = NodeState::LEADER;
        
        // Initialize leader state
        for (const auto& peer : peer_ids_) {
            next_index_[peer] = log_.size();
            match_index_[peer] = 0;
        }
    }
    
    int64_t get_current_time_ms() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
    }
    
    int get_election_timeout_ms() {
        std::uniform_int_distribution<int> dist(
            config_.election_timeout_min_ms,
            config_.election_timeout_max_ms
        );
        return dist(gen_);
    }
};

} // namespace dnn::distributed
