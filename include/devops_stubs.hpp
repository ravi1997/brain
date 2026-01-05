#pragma once
#include <iostream>

namespace dnn {
namespace devops {

// Mock TBB (Items 116, 119, 150-153)
class TBBWrapper {
public:
    static void parallel_invoke(auto f1, auto f2) {
        // Stub: Serial execution
        f1();
        f2();
    }
};

// Mock WebGPU (Items 157, 158)
class WebGPUContext {
public:
    void init() {
        std::cout << "[WebGPU] Stub Init" << std::endl;
    }
    
    void compute_shader(const std::string& shader) {
        // Stub
    }
};

// Mock gRPC (Items 159-162)
class GRPCStub {
public:
    void send_request() {}
};

}
}
