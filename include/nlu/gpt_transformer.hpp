#pragma once
#include <string>
#include <vector>

namespace dnn::nlu {

// GPT-style Transformer for text generation
class GPTTransformer {
public:
    GPTTransformer(int vocab_size = 50000, int d_model = 768, int num_layers = 12)
        : vocab_size_(vocab_size), d_model_(d_model), num_layers_(num_layers) {}
    
    // Generate next token (simplified)
    int generate_next_token(const std::vector<int>& context) {
        // Simple hash-based generation for demonstration
        if (context.empty()) return 0;
        
        size_t hash = 0;
        for (int token : context) {
            hash = hash * 31 + token;
        }
        
        return hash % vocab_size_;
    }
    
    // Generate text given context
    std::string generate(const std::string& prompt, int max_length = 50) {
        std::vector<int> tokens = tokenize(prompt);
        
        for (int i = 0; i < max_length; i++) {
            int next_token = generate_next_token(tokens);
            tokens.push_back(next_token);
            
            // Stop token (simplified)
            if (next_token == 0) break;
        }
        
        return detokenize(tokens);
    }
    
private:
    int vocab_size_;
    int d_model_;
    int num_layers_;
    
    std::vector<int> tokenize(const std::string& text) {
        std::vector<int> tokens;
        for (char c : text) {
            tokens.push_back(static_cast<int>(c));
        }
        return tokens;
    }
    
    std::string detokenize(const std::vector<int>& tokens) {
        std::string text;
        for (int token : tokens) {
            if (token >= 32 && token < 127) {
                text += static_cast<char>(token);
            }
        }
        return text;
    }
};

} // namespace dnn::nlu
