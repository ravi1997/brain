#pragma once
#include <chrono>
#include <functional>
#include <iostream>
#include <string>

namespace dnn {

class Profiler {
public:
    static double measure_latency(std::function<void()> func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }

    static void profile(const std::string& name, std::function<void()> func) {
        double ms = measure_latency(func);
        std::cout << "[PROFILE] " << name << ": " << ms << " ms" << std::endl;
    }
};

} // namespace dnn
