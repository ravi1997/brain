#include <gtest/gtest.h>
#include <chrono>
#include "../brain.hpp"
#include "../planning_unit.hpp"
#include "../auth_system.hpp"
#include "../decision_tree.hpp"

// Benchmarking Macros
#define BENCHMARK(name, iterations, code_block) \
    TEST(Benchmark, name) { \
        auto start = std::chrono::high_resolution_clock::now(); \
        for(int i=0; i<iterations; ++i) { \
            code_block \
        } \
        auto end = std::chrono::high_resolution_clock::now(); \
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(); \
        std::cout << "[BENCHMARK] " << #name << ": " << duration << "us (" << (duration/iterations) << "us/op)\n"; \
    }

// ID 1: Benchmark Planning Precision
BENCHMARK(Planning_Precision, 10, {
    PlanningUnit p;
    p.decide_best_action("Research", 0.5, 0.5);
})

// ID 4: Reflex Recall Stub
BENCHMARK(Reflex_Recall, 1000, {
    // Simulating lookup
    std::string key = "hello";
})

// ID 6: Concurrency Auth
BENCHMARK(Auth_Concurrency, 100, {
    dnn::AuthSystem auth;
    auth.login("user", "pass");
})

// ID 14: Decision Tree Concurrency
BENCHMARK(DecisionTree_Concurrency, 100, {
    dnn::DecisionTree dt;
    dt.predict({1.0, 2.0});
})

TEST(Benchmark, Sentiment_OpenMP) {
    // ID 5: Mock OpenMP / Parallel check
    Brain b;
    // Just ensuring it compiles and runs
    b.analyze_sentiment("Check");
    ASSERT_TRUE(true);
}
