#include "brain.hpp"
#include "brain_server.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <signal.h>
#include <execinfo.h> // For stack trace
#include <unistd.h>
#include <fcntl.h>

#include "crash_reporter.hpp"

int main() {
    CrashReporter::init("state/logs");
    
    std::cout << "Initializing Brain Replica..." << std::endl;
    
    try {
        Brain brain;
        BrainServer server(brain);
        server.start();

        std::cout << "Brain initialized. Multi-Port Server Active." << std::endl;
        std::cout << "Port 9005: Chat" << std::endl;
        std::cout << "Port 9001-9009: Active" << std::endl;

        // Auto-Train in background if vocab is empty (First run)
        std::thread([&brain]() {
            if (brain.get_knowledge_size() < 100) {
                 std::cout << "[System]: Brain appears empty. Initiating basic English download..." << std::endl;
                 std::ifstream basics("data/english_basics.txt");
                 if (basics) {
                     std::string line;
                     while(std::getline(basics, line)) {
                         size_t delimiter = line.find("|");
                         if (delimiter != std::string::npos) {
                             brain.teach(line.substr(0, delimiter), line.substr(delimiter + 1));
                         }
                     }
                     std::cout << "[System]: Basic English installed." << std::endl;
                 }
            }
        }).detach();

        // Launch Console Input Thread
        std::thread console_thread([&brain]() {
            std::string input;
            while (true) {
                std::getline(std::cin, input);
                if (input == "exit" || input == "quit") {
                    exit(0);
                }
                if (!input.empty()) {
                    std::string response = brain.interact(input);
                    std::cout << "\n[Brain]: " << response << "\n> " << std::flush;
                }
            }
        });
        console_thread.detach();

        // Main thread loop (keep alive)
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            // Maybe emit a dashboard heartbeat
            server.dash_server->broadcast("{\"status\": \"alive\", \"time\": \"" + std::to_string(std::time(nullptr)) + "\"}");
        }

    } catch (const std::exception& e) {
        std::cerr << "Critical Failure: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
