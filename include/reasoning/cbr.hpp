#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace dnn::reasoning {

// Case-Based Reasoning - retrieve and adapt similar past cases
class CaseBasedReasoning {
public:
    struct Case {
        std::string problem_description;
        std::vector<float> features;
        std::string solution;
        float success_rate;
        
        Case() : success_rate(0) {}
        Case(const std::string& prob, const std::vector<float>& feat, 
             const std::string& sol, float success = 1.0f)
            : problem_description(prob), features(feat), solution(sol), 
              success_rate(success) {}
    };
    
    CaseBasedReasoning() {}
    
    // Add case to case library
    void add_case(const Case& c) {
        case_library_.push_back(c);
    }
    
    // Retrieve most similar cases
    std::vector<Case> retrieve(const std::vector<float>& query_features, int k = 5) {
        if (case_library_.empty()) {
            return {};
        }
        
        // Compute similarities
        std::vector<std::pair<float, int>> similarities;
        
        for (size_t i = 0; i < case_library_.size(); i++) {
            float sim = compute_similarity(query_features, case_library_[i].features);
            similarities.emplace_back(sim, i);
        }
        
        // Sort by similarity (descending)
        std::sort(similarities.begin(), similarities.end(),
                 [](const auto& a, const auto& b) { return a.first > b.first; });
        
        // Return top k
        std::vector<Case> retrieved;
        for (int i = 0; i < k && i < static_cast<int>(similarities.size()); i++) {
            retrieved.push_back(case_library_[similarities[i].second]);
        }
        
        return retrieved;
    }
    
    // Reuse: adapt solution from similar case
    std::string reuse(const std::vector<Case>& similar_cases,
                     const std::vector<float>& query_features) {
        if (similar_cases.empty()) {
            return "No similar cases found";
        }
        
        // Weighted voting based on similarity and success rate
        std::unordered_map<std::string, float> solution_scores;
        
        for (const auto& c : similar_cases) {
            float similarity = compute_similarity(query_features, c.features);
            float score = similarity * c.success_rate;
            solution_scores[c.solution] += score;
        }
        
        // Find best solution
        std::string best_solution;
        float best_score = 0.0f;
        
        for (const auto& [solution, score] : solution_scores) {
            if (score > best_score) {
                best_score = score;
                best_solution = solution;
            }
        }
        
        return best_solution;
    }
    
    // Revise: adjust solution based on feedback
    void revise(Case& adapted_case, float actual_success) {
        adapted_case.success_rate = actual_success;
    }
    
    // Retain: store new case in library
    void retain(const Case& new_case) {
        add_case(new_case);
    }
    
    // Complete CBR cycle: Retrieve, Reuse, Revise, Retain
    std::string solve(const std::vector<float>& problem_features,
                     const std::string& problem_desc) {
        // Retrieve
        auto similar_cases = retrieve(problem_features, 5);
        
        // Reuse
        std::string solution = reuse(similar_cases, problem_features);
        
        return solution;
    }
    
private:
    std::vector<Case> case_library_;
    
    float compute_similarity(const std::vector<float>& a, const std::vector<float>& b) {
        // Cosine similarity
        if (a.empty() || b.empty()) return 0.0f;
        
        float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
        size_t min_size = std::min(a.size(), b.size());
        
        for (size_t i = 0; i < min_size; i++) {
            dot += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }
        
        if (norm_a == 0 || norm_b == 0) return 0.0f;
        
        return dot / (std::sqrt(norm_a) * std::sqrt(norm_b));
    }
};

} // namespace dnn::reasoning
