#pragma once
#include <vector>
#include <mutex>

namespace dnn {
    class DecisionTree {
    public:
        struct Node {
             int id;
             bool leaf;
             double value;
        };

        double predict(const std::vector<double>& input) {
            std::lock_guard<std::mutex> lock(mu_);
            // Stub prediction
            return 0.5;
        }
    private:
        std::mutex mu_;
    };
}
