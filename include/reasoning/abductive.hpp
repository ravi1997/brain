#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace dnn::reasoning {

// Abductive Reasoning - inference to the best explanation
class AbductiveReasoning {
public:
    struct Observation {
        std::string fact;
        float confidence;
        
        Observation() : confidence(1.0f) {}
        Observation(const std::string& f, float c = 1.0f) : fact(f), confidence(c) {}
    };
    
    struct Hypothesis {
        std::string explanation;
        std::vector<std::string> assumptions;
        float prior_probability;
        float likelihood;  // P(observations | hypothesis)
        float posterior;   // P(hypothesis | observations)
        
        Hypothesis() : prior_probability(0.5f), likelihood(0.5f), posterior(0.5f) {}
    };
    
    struct Rule {
        std::vector<std::string> conditions;
        std::string conclusion;
        float confidence;
        
        Rule() : confidence(1.0f) {}
    };
    
    AbductiveReasoning() {}
    
    // Add background knowledge rule
    void add_rule(const Rule& rule) {
        rules_.push_back(rule);
    }
    
    // Generate hypotheses that explain observations
    std::vector<Hypothesis> generate_hypotheses(const std::vector<Observation>& observations) {
        std::vector<Hypothesis> hypotheses;
        
        // For each observation, find rules that could produce it
        for (const auto& obs : observations) {
            for (const auto& rule : rules_) {
                if (rule.conclusion == obs.fact) {
                    // Create hypothesis from rule conditions
                    Hypothesis hyp;
                    hyp.explanation = "If " + join(rule.conditions, " and ") + 
                                     " then " + rule.conclusion;
                    hyp.assumptions = rule.conditions;
                    hyp.prior_probability = rule.confidence;
                    hypotheses.push_back(hyp);
                }
            }
            
            // Also create simple hypothesis: observation is given fact
            Hypothesis direct_hyp;
            direct_hyp.explanation = obs.fact + " is simply true";
            direct_hyp.assumptions = {obs.fact};
            direct_hyp.prior_probability = 0.3f;
            hypotheses.push_back(direct_hyp);
        }
        
        return hypotheses;
    }
    
    // Find best explanation using Bayesian inference
    Hypothesis abduce(const std::vector<Observation>& observations) {
        auto hypotheses = generate_hypotheses(observations);
        
        if (hypotheses.empty()) {
            return Hypothesis();
        }
        
        // Compute posterior for each hypothesis
        for (auto& hyp : hypotheses) {
            // Likelihood: how well does hypothesis explain observations?
            hyp.likelihood = compute_likelihood(hyp, observations);
            
            // Posterior ∝ Likelihood × Prior
            hyp.posterior = hyp.likelihood * hyp.prior_probability;
        }
        
        // Normalize posteriors
        float total_posterior = 0.0f;
        for (const auto& hyp : hypotheses) {
            total_posterior += hyp.posterior;
        }
        
        if (total_posterior > 0) {
            for (auto& hyp : hypotheses) {
                hyp.posterior /= total_posterior;
            }
        }
        
        // Return hypothesis with highest posterior
        auto best = std::max_element(hypotheses.begin(), hypotheses.end(),
                                     [](const Hypothesis& a, const Hypothesis& b) {
                                         return a.posterior < b.posterior;
                                     });
        
        return *best;
    }
    
    // Find minimal set of assumptions that explain observations
    std::vector<std::string> minimal_explanation(const std::vector<Observation>& observations) {
        std::unordered_set<std::string> all_assumptions;
        
        auto hypotheses = generate_hypotheses(observations);
        
        // Collect all assumptions
        for (const auto& hyp : hypotheses) {
            for (const auto& assumption : hyp.assumptions) {
                all_assumptions.insert(assumption);
            }
        }
        
        // Greedy set cover: find minimal set that explains all observations
        std::unordered_set<std::string> covered_observations;
        std::vector<std::string> minimal_set;
        
        while (covered_observations.size() < observations.size() && 
               !all_assumptions.empty()) {
            // Find assumption that covers most uncovered observations
            std::string best_assumption;
            int best_coverage = 0;
            
            for (const auto& assumption : all_assumptions) {
                int coverage = count_coverage(assumption, observations, covered_observations);
                if (coverage > best_coverage) {
                    best_coverage = coverage;
                    best_assumption = assumption;
                }
            }
            
            if (best_coverage == 0) {
                break;
            }
            
            minimal_set.push_back(best_assumption);
            all_assumptions.erase(best_assumption);
            
            // Update covered observations
            update_coverage(best_assumption, observations, covered_observations);
        }
        
        return minimal_set;
    }
    
private:
    std::vector<Rule> rules_;
    
    float compute_likelihood(const Hypothesis& hyp, const std::vector<Observation>& observations) {
        // How many observations are explained by this hypothesis?
        int explained_count = 0;
        
        for (const auto& obs : observations) {
            // Check if hypothesis can produce this observation
            for (const auto& rule : rules_) {
                bool all_conditions_met = true;
                for (const auto& cond : rule.conditions) {
                    if (std::find(hyp.assumptions.begin(), hyp.assumptions.end(), cond) == 
                        hyp.assumptions.end()) {
                        all_conditions_met = false;
                        break;
                    }
                }
                
                if (all_conditions_met && rule.conclusion == obs.fact) {
                    explained_count++;
                    break;
                }
            }
        }
        
        return observations.empty() ? 0.0f : 
               static_cast<float>(explained_count) / observations.size();
    }
    
    int count_coverage(const std::string& assumption,
                      const std::vector<Observation>& observations,
                      const std::unordered_set<std::string>& already_covered) {
        int count = 0;
        
        for (const auto& obs : observations) {
            if (already_covered.count(obs.fact)) {
                continue;
            }
            
            // Check if this assumption helps explain the observation
            for (const auto& rule : rules_) {
                if (std::find(rule.conditions.begin(), rule.conditions.end(), assumption) != 
                    rule.conditions.end() && rule.conclusion == obs.fact) {
                    count++;
                    break;
                }
            }
        }
        
        return count;
    }
    
    void update_coverage(const std::string& assumption,
                        const std::vector<Observation>& observations,
                        std::unordered_set<std::string>& covered) {
        for (const auto& obs : observations) {
            for (const auto& rule : rules_) {
                if (std::find(rule.conditions.begin(), rule.conditions.end(), assumption) !=
                    rule.conditions.end() && rule.conclusion == obs.fact) {
                    covered.insert(obs.fact);
                }
            }
        }
    }
    
    std::string join(const std::vector<std::string>& vec, const std::string& delim) const {
        std::string result;
        for (size_t i = 0; i < vec.size(); i++) {
            result += vec[i];
            if (i < vec.size() - 1) {
                result += delim;
            }
        }
        return result;
    }
};

} // namespace dnn::reasoning
