#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <iostream>

namespace dnn {

class FederationUnit {
public:
    struct Fact {
        std::string subject;
        std::string predicate;
        std::string object;
        double confidence;
        std::string source_id;
    };

    void propose_fact(const Fact& f) {
        std::lock_guard<std::mutex> lock(mu_);
        pending_facts_.push_back(f);
    }

    std::vector<Fact> sync_knowledge() {
        std::lock_guard<std::mutex> lock(mu_);
        // Simulation: Accept all facts with > 0.8 confidence
        std::vector<Fact> accepted;
        for (const auto& f : pending_facts_) {
            if (f.confidence > 0.8) {
                accepted.push_back(f);
                global_graph_.push_back(f);
            }
        }
        pending_facts_.clear();
        return accepted;
    }

private:
    std::mutex mu_;
    std::vector<Fact> pending_facts_;
    std::vector<Fact> global_graph_; // Local cache of global knowledge
};

} // namespace dnn
