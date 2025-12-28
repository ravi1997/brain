#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>

namespace dnn::nlu {

// BERT-style contextualized word embeddings
class BERTEmbeddings {
public:
    BERTEmbeddings(int vocab_size = 30000, int embedding_dim = 768, int max_seq_len = 512)
        : vocab_size_(vocab_size), embedding_dim_(embedding_dim), max_seq_len_(max_seq_len) {
        
        // Initialize token embeddings
        token_embeddings_.resize(vocab_size * embedding_dim);
        for (auto& e : token_embeddings_) {
            e = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.02f;
        }
        
        // Initialize position embeddings
        position_embeddings_.resize(max_seq_len * embedding_dim);
        for (auto& e : position_embeddings_) {
            e = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.02f;
        }
    }
    
    // Get embedding for a token ID
    std::vector<float> get_token_embedding(int token_id) const {
        if (token_id < 0 || token_id >= vocab_size_) {
            return std::vector<float>(embedding_dim_, 0.0f);
        }
        
        int offset = token_id * embedding_dim_;
        return std::vector<float>(token_embeddings_.begin() + offset,
                                 token_embeddings_.begin() + offset + embedding_dim_);
    }
    
    // Get combined embedding (token + position)
    std::vector<float> get_embedding(int token_id, int position) const {
        auto token_emb = get_token_embedding(token_id);
        
        if (position < 0 || position >= max_seq_len_) {
            return token_emb;
        }
        
        // Add position embedding
        int pos_offset = position * embedding_dim_;
        for (int i = 0; i < embedding_dim_; i++) {
            token_emb[i] += position_embeddings_[pos_offset + i];
        }
        
        return token_emb;
    }
    
    // Encode a sequence of token IDs
    std::vector<std::vector<float>> encode_sequence(const std::vector<int>& token_ids) const {
        std::vector<std::vector<float>> embeddings;
        embeddings.reserve(token_ids.size());
        
        for (size_t i = 0; i < token_ids.size(); i++) {
            embeddings.push_back(get_embedding(token_ids[i], i));
        }
        
        return embeddings;
    }
    
    // Simple tokenization (space-based)
    std::vector<int> tokenize(const std::string& text) const {
        std::vector<int> tokens;
        std::string word;
        
        for (char c : text) {
            if (c == ' ' || c == '\n' || c == '\t') {
                if (!word.empty()) {
                    tokens.push_back(get_token_id(word));
                    word.clear();
                }
            } else {
                word += c;
            }
        }
        
        if (!word.empty()) {
            tokens.push_back(get_token_id(word));
        }
        
        return tokens;
    }
    
private:
    int vocab_size_;
    int embedding_dim_;
    int max_seq_len_;
    std::vector<float> token_embeddings_;
    std::vector<float> position_embeddings_;
    
    int get_token_id(const std::string& token) const {
        // Simple hash-based token ID
        size_t hash = 0;
        for (char c : token) {
            hash = hash * 31 + c;
        }
        return hash % vocab_size_;
    }
};

} // namespace dnn::nlu
