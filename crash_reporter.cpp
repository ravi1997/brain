#include "crash_reporter.hpp"
#include <csignal>
#include <fstream>
#include <iostream>
#include <ctime>
#include <filesystem>
#include <execinfo.h>
#include <unistd.h>
#include <cstring>
#include <vector>

std::string CrashReporter::log_directory = "./";

void CrashReporter::init(const std::string& log_dir) {
    log_directory = log_dir;
    std::filesystem::create_directories(log_directory);

    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::signal(SIGFPE, signal_handler);
}

void CrashReporter::signal_handler(int signal) {
    std::time_t result = std::time(nullptr);
    char t_str[100];
    if (std::strftime(t_str, sizeof(t_str), "%Y-%m-%d_%H-%M-%S", std::localtime(&result))) {
        std::string filename = CrashReporter::log_directory + "/crash_" + t_str + ".log";
        std::ofstream outfile(filename);
        
        outfile << "Crash detected! Signal: " << signal << " (" << strsignal(signal) << ")" << std::endl;
        
        // Stack Trace
        void* array[20];
        size_t size = backtrace(array, 20);
        char** strings = backtrace_symbols(array, size);

        outfile << "Stack Trace:" << std::endl;
        if (strings) {
            for (size_t i = 0; i < size; i++) {
                outfile << strings[i] << std::endl;
            }
            free(strings);
        }
        
        outfile.close();
        std::cerr << "[CRASH] Signal " << signal << " caught. Log saved to " << filename << std::endl;
    }
    
    // Restore default handler and raise again to terminate
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}
