#pragma once
#include <string>

class CrashReporter {
public:
    static void init(const std::string& log_dir);
    
private:
    static void signal_handler(int signal);
    static std::string log_directory;
};
