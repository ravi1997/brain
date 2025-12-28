#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace dnn::infra {

// Dense Retrieval using vector embeddings
class DenseRetrieval {
public:
    struct Document {
        std::string id;
        std::string content;
        std::vector<float> embedding;
        float score;
        
        Document() : score(0.0f) {}
        Document(const std::string& doc_id, const std::string& text,
                const std::vector<float>& emb = {})
            : id(doc_id), content(text), embedding(emb), score(0.0f) {}
    };
    
    DenseRetrieval(int embedding_dim = 768) : embedding_dim_(embedding_dim) {}
    
    // Index a document
    void index_document(const std::string& doc_id, const std::string& content,
                       const std::vector<float>& embedding) {
        documents_[doc_id] = Document(doc_id, content, embedding);
    }
    
    // Retrieve top-k documents for a query
    std::vector<Document> retrieve(const std::vector<float>& query_embedding, int top_k = 10) {
        std::vector<Document> scored_docs;
        
        for (auto& [id, doc] : documents_) {
            doc.score = cosine_similarity(query_embedding, doc.embedding);
            scored_docs.push_back(doc);
        }
        
        // Sort by score descending
        std::partial_sort(scored_docs.begin(),
                         scored_docs.begin() + std::min(top_k, static_cast<int>(scored_docs.size())),
                         scored_docs.end(),
                         [](const Document& a, const Document& b) { return a.score > b.score; });
        
        scored_docs.resize(std::min(top_k, static_cast<int>(scored_docs.size())));
        return scored_docs;
    }
    
    // Get document by ID
    Document get_document(const std::string& doc_id) const {
        auto it = documents_.find(doc_id);
        return it != documents_.end() ? it->second : Document();
    }
    
    // Remove document
    void remove_document(const std::string& doc_id) {
        documents_.erase(doc_id);
    }
    
    // Get document count
    size_t size() const {
        return documents_.size();
    }
    
private:
    int embedding_dim_;
    std::unordered_map<std::string, Document> documents_;
    
    float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) const {
        if (a.size() != b.size() || a.empty()) return 0.0f;
        
        float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
        for (size_t i = 0; i < a.size(); i++) {
            dot += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }
        
        float denom = std::sqrt(norm_a) * std::sqrt(norm_b);
        return denom > 0 ? dot / denom : 0.0f;
    }
};

} // namespace dnn::infra
