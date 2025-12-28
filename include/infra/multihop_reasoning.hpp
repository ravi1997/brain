#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace dnn::infra {

class MultiHopReasoning {
public:
    struct Fact {
        std::string subject;
        std::string predicate;
        std::string object;
    };
    
    MultiHopReasoning() {}
    
    void add_fact(const std::string& subj, const std::string& pred, const std::string& obj) {
        facts_.push_back({subj, pred, obj});
        graph_[subj].push_back({pred, obj});
    }
    
    std::vector<std::vector<std::string>> find_paths(
        const std::string& start, const std::string& end, int max_hops = 3) {
        std::vector<std::vector<std::string>> paths;
        std::vector<std::string> current_path = {start};
        dfs_paths(start, end, max_hops, current_path, paths);
        return paths;
    }
    
    std::vector<std::string> query(const std::string& subj, const std::string& pred, int max_hops = 2) {
        std::unordered_set<std::string> results;
        for (const auto& [relation, target] : graph_[subj]) {
            if (relation == pred) {
                results.insert(target);
            }
        }
        return {results.begin(), results.end()};
    }
    
private:
    std::vector<Fact> facts_;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> graph_;
    
    void dfs_paths(const std::string& current, const std::string& target, int remaining_hops,
                  std::vector<std::string>& path, std::vector<std::vector<std::string>>& all_paths) {
        if (current == target) {
            all_paths.push_back(path);
            return;
        }
        if (remaining_hops == 0) return;
        if (graph_.count(current)) {
            for (const auto& [relation, next] : graph_[current]) {
                path.push_back(next);
                dfs_paths(next, target, remaining_hops - 1, path, all_paths);
                path.pop_back();
            }
        }
    }
};

} // namespace dnn::infra
