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

void crash_handler(int sig) {
    void *array[20];
    size_t size = backtrace(array, 20);
    
    // Write to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    
    // Write to crash.log (Signal-safe open)
    int fd = open("crash.log", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd != -1) {
        const char* msg = "CRASH DETECTED. Signal: ";
        write(fd, msg, 24);
        // Rough int to string to allow no malloc
        char buf[10];
        int len = snprintf(buf, 10, "%d\n", sig);
        write(fd, buf, len);
        
        backtrace_symbols_fd(array, size, fd);
        close(fd);
    }
    
    exit(1);
}

int main() {
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
    
    std::cout << "Initializing Brain Replica..." << std::endl;
    
    try {
        Brain brain;
        BrainServer server(brain);
        server.start();

        std::cout << "Brain initialized. Multi-Port Server Active." << std::endl;
        std::cout << "Port 9005: Chat" << std::endl;
        std::cout << "Port 9001-9009: Active" << std::endl;

        // Auto-Train if vocab is empty (First run)
        if (brain.get_knowledge_size() < 100) {
             std::cout << "[System]: Brain appears empty. Initiating basic English download..." << std::endl;
             std::ifstream basics("english_basics.txt");
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
