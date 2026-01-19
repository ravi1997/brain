#include <iostream>
#include <cassert>
#include "cognitive_engine.hpp"

int main() {
    std::cout << "Testing CognitiveEngine..." << std::endl;

    dnn::CognitiveEngine engine(dnn::EngineType::MOCK_FILE);

    // Test 1: Ask
    dnn::CognitiveResponse response = engine.ask("Hello Brain");
    std::cout << "Response: " << response.content << std::endl;
    assert(response.content.find("processed") != std::string::npos);

    // Test 2: Plan
    std::vector<std::string> plan = engine.plan_task("Build a spaceship");
    assert(!plan.empty());
    std::cout << "Plan Step 1: " << plan[0] << std::endl;

    // Test 3: Reflect
    std::string reflection = engine.reflect("I failed to build the spaceship");
    assert(reflection.find("processed") != std::string::npos); // Mock returns same generic response often

    std::cout << "CognitiveEngine Tests Parsed!" << std::endl;
    return 0;
}
