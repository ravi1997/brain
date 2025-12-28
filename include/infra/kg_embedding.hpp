#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <random>
#include <cmath>
#include <algorithm>

namespace dnn::infra {

// TransE: Translating Embeddings for Knowledge Graph Completion
class KnowledgeGraphEmbedding {
public:
    struct Triple {
        std::string head;
        std::string relation;
        std::string tail;
        
        Triple(const std::string& h, const std::string& r, const std::string& t)
            : head(h), relation(r), tail(t) {}
    };
    
    KnowledgeGraphEmbedding(int embedding_dim = 50, float learning_rate = 0.01f, float margin = 1.0f)
        : dim_(embedding_dim), lr_(learning_rate), margin_(margin), gen_(std::random_device{}()) {}
    
    // Train the model on triples
    void train(const std::vector<Triple>& triples, int epochs = 100, int negative_samples = 10) {
        // Initialize embeddings
        initialize_embeddings(triples);
        
        for (int epoch = 0; epoch < epochs; epoch++) {
            float total_loss = 0.0f;
            
            for (const auto& triple : triples) {
                // Positive sample
                auto& h = entity_embeddings_[triple.head];
                auto& r = relation_embeddings_[triple.relation];
                auto& t = entity_embeddings_[triple.tail];
                
                float pos_score = score_triple(h, r, t);
                
                // Negative samples
                for (int i = 0; i < negative_samples; i++) {
                    Triple neg_triple = corrupt_triple(triple, triples);
                    auto& h_neg = entity_embeddings_[neg_triple.head];
                    auto& t_neg = entity_embeddings_[neg_triple.tail];
                    
                    float neg_score = score_triple(h_neg, r, t_neg);
                    
                    // Margin loss: max(0, margin + pos_score - neg_score)
                    float loss = std::max(0.0f, margin_ + pos_score - neg_score);
                    total_loss += loss;
                    
                    if (loss > 0) {
                        // Update embeddings
                        update_embeddings(h, r, t, h_neg, t_neg, triple.head == neg_triple.head);
                    }
                }
            }
            
            // Normalize embeddings
            normalize_embeddings();
        }
    }
    
    // Score a triple (lower is better)
    float score(const std::string& head, const std::string& relation, const std::string& tail) const {
        auto h_it = entity_embeddings_.find(head);
        auto r_it = relation_embeddings_.find(relation);
        auto t_it = entity_embeddings_.find(tail);
        
        if (h_it == entity_embeddings_.end() || 
            r_it == relation_embeddings_.end() || 
            t_it == entity_embeddings_.end()) {
            return std::numeric_limits<float>::max();
        }
        
        return score_triple(h_it->second, r_it->second, t_it->second);
    }
    
    // Predict tail entity given head and relation
    std::vector<std::pair<std::string, float>> predict_tail(
        const std::string& head, const std::string& relation, int top_k = 10) const {
        
        auto h_it = entity_embeddings_.find(head);
        auto r_it = relation_embeddings_.find(relation);
        
        if (h_it == entity_embeddings_.end() || r_it == relation_embeddings_.end()) {
            return {};
        }
        
        std::vector<std::pair<std::string, float>> candidates;
        
        for (const auto& [entity, embedding] : entity_embeddings_) {
            float s = score_triple(h_it->second, r_it->second, embedding);
            candidates.emplace_back(entity, s);
        }
        
        // Sort by score (lower is better)
        std::partial_sort(
            candidates.begin(),
            candidates.begin() + std::min(top_k, static_cast<int>(candidates.size())),
            candidates.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; }
        );
        
        candidates.resize(std::min(top_k, static_cast<int>(candidates.size())));
        return candidates;
    }
    
    // Get embedding for entity
    std::vector<float> get_entity_embedding(const std::string& entity) const {
        auto it = entity_embeddings_.find(entity);
        return it != entity_embeddings_.end() ? it->second : std::vector<float>(dim_, 0.0f);
    }
    
private:
    int dim_;
    float lr_;
    float margin_;
    std::mt19937 gen_;
    std::unordered_map<std::string, std::vector<float>> entity_embeddings_;
    std::unordered_map<std::string, std::vector<float>> relation_embeddings_;
    
    void initialize_embeddings(const std::vector<Triple>& triples) {
        std::uniform_real_distribution<float> dist(-6.0f / std::sqrt(dim_), 6.0f / std::sqrt(dim_));
        
        for (const auto& triple : triples) {
            if (entity_embeddings_.find(triple.head) == entity_embeddings_.end()) {
                entity_embeddings_[triple.head] = random_vector(dist);
            }
            if (entity_embeddings_.find(triple.tail) == entity_embeddings_.end()) {
                entity_embeddings_[triple.tail] = random_vector(dist);
            }
            if (relation_embeddings_.find(triple.relation) == relation_embeddings_.end()) {
                relation_embeddings_[triple.relation] = random_vector(dist);
            }
        }
    }
    
    std::vector<float> random_vector(std::uniform_real_distribution<float>& dist) {
        std::vector<float> v(dim_);
        for (auto& x : v) {
            x = dist(gen_);
        }
        return v;
    }
    
    // Score: ||h + r - t||
    float score_triple(const std::vector<float>& h, const std::vector<float>& r, 
                      const std::vector<float>& t) const {
        float sum = 0.0f;
        for (int i = 0; i < dim_; i++) {
            float diff = h[i] + r[i] - t[i];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }
    
    Triple corrupt_triple(const Triple& triple, const std::vector<Triple>& all_triples) {
        std::uniform_int_distribution<int> coin(0, 1);
        std::uniform_int_distribution<int> entity_dist(0, entity_embeddings_.size() - 1);
        
        if (coin(gen_) == 0) {
            // Corrupt head
            auto it = entity_embeddings_.begin();
            std::advance(it, entity_dist(gen_));
            return Triple(it->first, triple.relation, triple.tail);
        } else {
            // Corrupt tail
            auto it = entity_embeddings_.begin();
            std::advance(it, entity_dist(gen_));
            return Triple(triple.head, triple.relation, it->first);
        }
    }
    
    void update_embeddings(std::vector<float>& h, std::vector<float>& r, std::vector<float>& t,
                          std::vector<float>& h_neg, std::vector<float>& t_neg, bool corrupt_head) {
        // Gradient for positive triple
        std::vector<float> grad(dim_);
        for (int i = 0; i < dim_; i++) {
            grad[i] = 2 * (h[i] + r[i] - t[i]);
        }
        
        // Update positive embeddings
        for (int i = 0; i < dim_; i++) {
            h[i] -= lr_ * grad[i];
            r[i] -= lr_ * grad[i];
            t[i] += lr_ * grad[i];
        }
        
        // Gradient for negative triple
        if (corrupt_head) {
            for (int i = 0; i < dim_; i++) {
                float grad_neg = 2 * (h_neg[i] + r[i] - t[i]);
                h_neg[i] += lr_ * grad_neg;
            }
        } else {
            for (int i = 0; i < dim_; i++) {
                float grad_neg = 2 * (h[i] + r[i] - t_neg[i]);
                t_neg[i] -= lr_ * grad_neg;
            }
        }
    }
    
    void normalize_embeddings() {
        for (auto& [_, embedding] : entity_embeddings_) {
            float norm = 0.0f;
            for (float x : embedding) {
                norm += x * x;
            }
            norm = std::sqrt(norm);
            if (norm > 1.0f) {
                for (float& x : embedding) {
                    x /= norm;
                }
            }
        }
    }
};

} // namespace dnn::infra
