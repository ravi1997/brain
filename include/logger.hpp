#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <filesystem>
#include <iomanip>
#include <chrono>
#include <iostream>

class Logger {
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    void init(const std::string& filename, size_t max_size_bytes = 5 * 1024 * 1024, int max_backups = 3) {
        std::lock_guard<std::mutex> lock(mutex_);
        filename_ = filename;
        max_size_ = max_size_bytes;
        max_backups_ = max_backups;
        
        // Open file in append mode
        file_.open(filename_, std::ios::app);
    }

    void log(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!file_.is_open()) return;

        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        
        file_ << "[" << std::put_time(std::localtime(&now_c), "%F %T") << "] " << msg << std::endl;
        
        // Check size and rotate if needed
        if (file_.tellp() > static_cast<std::streamoff>(max_size_)) {
            rotate();
        }
    }

private:
    Logger() = default;
    ~Logger() {
        if (file_.is_open()) file_.close();
    }

    void rotate() {
        file_.close();

        // Rotate existing backups
        // file.log.2 -> file.log.3
        for (int i = max_backups_ - 1; i >= 1; --i) {
            std::string old_name = filename_ + "." + std::to_string(i);
            std::string new_name = filename_ + "." + std::to_string(i + 1);
            if (std::filesystem::exists(old_name)) {
                std::filesystem::rename(old_name, new_name);
            }
        }
        
        // file.log -> file.log.1
        std::string backup_name = filename_ + ".1";
        if (std::filesystem::exists(filename_)) {
            std::filesystem::rename(filename_, backup_name);
        }

        // Reopen main file
        file_.open(filename_, std::ios::out); // New file
    }

    std::string filename_;
    std::ofstream file_;
    std::mutex mutex_;
    size_t max_size_ = 5 * 1024 * 1024;
    int max_backups_ = 3;
};
