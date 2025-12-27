#pragma once
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <algorithm>
#include <sstream>

enum class TaskType {
    RESEARCH,
    SLEEP,
    INTERACTION,
    MAINTENANCE,
    IDLE
};

enum class TaskPriority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

struct Task {
    int id;
    std::string description;
    TaskType type;
    TaskPriority priority;
    std::string status; // "PENDING", "ACTIVE", "COMPLETED"
    
    std::string to_json() const {
        std::stringstream ss;
        ss << "{\"id\": " << id << ", \"desc\": \"" << description << "\", \"status\": \"" << status << "\", \"priority\": " << (int)priority << "}";
        return ss.str();
    }
};

class TaskManager {
public:
    TaskManager();
    
    void add_task(const std::string& desc, TaskType type, TaskPriority priority);
    Task* get_next_task(); // Returns pointer to active task or nullptr
    void complete_active_task();
    
    // For Monitoring
    std::string get_json_snapshot(); 
    bool has_pending_tasks(); // Mega-Batch 6

private:
    std::deque<Task> pending_queue;
    std::vector<Task> history; // Keep last 10
    Task* active_task = nullptr;
    int next_id = 1;
    std::mutex mutex_;
};
