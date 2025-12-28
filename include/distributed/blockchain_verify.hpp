#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace dnn::distributed {

// Blockchain verification and consensus
class BlockchainVerification {
public:
    struct Block {
        int index;
        int64_t timestamp;
        std::string data;
        std::string previous_hash;
        std::string hash;
        int nonce;
        
        Block() : index(0), timestamp(0), nonce(0) {}
        
        Block(int idx, const std::string& block_data, const std::string& prev_hash)
            : index(idx), data(block_data), previous_hash(prev_hash), nonce(0) {
            timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        }
    };
    
    BlockchainVerification(int difficulty = 2) : difficulty_(difficulty) {
        // Create genesis block
        Block genesis(0, "Genesis Block", "0");
        genesis.hash = calculate_hash(genesis);
        chain_.push_back(genesis);
    }
    
    // Add a new block to the chain
    bool add_block(const std::string& data) {
        Block new_block(chain_.size(), data, chain_.back().hash);
        
        // Mine the block (Proof of Work)
        mine_block(new_block);
        
        // Verify before adding
        if (is_valid_new_block(new_block, chain_.back())) {
            chain_.push_back(new_block);
            return true;
        }
        
        return false;
    }
    
    // Verify the entire blockchain
    bool verify_chain() const {
        // Check genesis block
        if (chain_.empty()) return false;
        
        Block genesis = chain_[0];
        if (genesis.previous_hash != "0") return false;
        if (genesis.hash != calculate_hash(genesis)) return false;
        
        // Check all subsequent blocks
        for (size_t i = 1; i < chain_.size(); i++) {
            if (!is_valid_new_block(chain_[i], chain_[i-1])) {
                return false;
            }
        }
        
        return true;
    }
    
    // Get the blockchain
    std::vector<Block> get_chain() const {
        return chain_;
    }
    
    // Get latest block
    Block get_latest_block() const {
        return chain_.empty() ? Block() : chain_.back();
    }
    
    // Calculate block hash
    std::string calculate_hash(const Block& block) const {
        std::stringstream ss;
        ss << block.index << block.timestamp << block.data 
           << block.previous_hash << block.nonce;
        
        // Simple hash function (in production, use SHA-256)
        std::string input = ss.str();
        size_t hash_value = 0;
        for (char c : input) {
            hash_value = hash_value * 31 + c;
        }
        
        std::stringstream hash_ss;
        hash_ss << std::hex << std::setfill('0') << std::setw(16) << hash_value;
        return hash_ss.str();
    }
    
private:
    int difficulty_;
    std::vector<Block> chain_;
    
    // Proof of Work mining
    void mine_block(Block& block) {
        std::string target(difficulty_, '0');
        
        while (true) {
            block.hash = calculate_hash(block);
            
            // Check if hash meets difficulty requirement
            if (block.hash.substr(0, difficulty_) == target) {
                break;
            }
            
            block.nonce++;
            
            // Limit nonce to prevent infinite loop in simulation
            if (block.nonce > 100000) {
                break;
            }
        }
    }
    
    // Validate a new block
    bool is_valid_new_block(const Block& new_block, const Block& previous_block) const {
        // Check index
        if (new_block.index != previous_block.index + 1) {
            return false;
        }
        
        // Check previous hash
        if (new_block.previous_hash != previous_block.hash) {
            return false;
        }
        
        // Check hash
        if (new_block.hash != calculate_hash(new_block)) {
            return false;
        }
        
        // Check proof of work
        std::string target(difficulty_, '0');
        if (new_block.hash.substr(0, difficulty_) != target) {
            return false;
        }
        
        return true;
    }
};

} // namespace dnn::distributed
