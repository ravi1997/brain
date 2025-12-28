#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include "../profiler.hpp"
#include "../auth_system.hpp"

// Stress Test 1: Concurrency on Auth System
TEST(StressTest, AuthConcurrency) {
    dnn::AuthSystem auth;
    std::atomic<int> success_count{0};
    int thread_count = 50; // High contention
    int loops = 100;

    std::vector<std::thread> threads;
    auto task = [&]() {
        for(int i=0; i<loops; ++i) {
            if (auth.login("user", "pass")) {
                success_count++;
            }
        }
    };

    dnn::Profiler::profile("Auth Stress", [&]() {
        for(int i=0; i<thread_count; ++i) threads.emplace_back(task);
        for(auto& t : threads) t.join();
    });

    ASSERT_EQ(success_count, thread_count * loops);
}

// Stress Test 2: Latency Profiler check
TEST(StressTest, ProfilingCheck) {
    double ms = dnn::Profiler::measure_latency([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
    ASSERT_GE(ms, 10.0);
}
