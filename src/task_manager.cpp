#include "task_manager.hpp"
#include <iostream>

TaskManager::TaskManager() {}

void TaskManager::add_task(const std::string& desc, TaskType type, TaskPriority priority) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Avoid duplicate pending tasks to prevent spam
    for(const auto& t : pending_queue) {
        if (t.description == desc && t.status == "PENDING") return;
    }

    Task t;
    t.id = next_id++;
    t.description = desc;
    t.type = type;
    t.priority = priority;
    t.status = "PENDING";
    
    pending_queue.push_back(t);
    
    // Sort by priority (High to Low)
    std::sort(pending_queue.begin(), pending_queue.end(), [](const Task& a, const Task& b) {
        return (int)a.priority > (int)b.priority;
    });
}

Task* TaskManager::get_next_task() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (active_task != nullptr) return active_task; // Busy

    if (pending_queue.empty()) return nullptr;

    // Move from pending to active (we keep it in pending deque for now but mark as active, 
    // or better, move to a separate member. For simplicity, let's say active_task points to a heap object or specific slot)
    // Actually, safest is to pop from pending and store in a dedicated active_task object.
    
    static Task current_execution; 
    current_execution = pending_queue.front();
    pending_queue.pop_front();
    
    current_execution.status = "ACTIVE";
    active_task = &current_execution;
    
    return active_task;
}

void TaskManager::complete_active_task() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (active_task) {
        active_task->status = "COMPLETED";
        history.push_back(*active_task);
        if (history.size() > 10) history.erase(history.begin());
        active_task = nullptr;
    }
}

std::string TaskManager::get_json_snapshot() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::stringstream ss;
    ss << "{";
    
    ss << "\"active\": ";
    if (active_task) ss << active_task->to_json();
    else ss << "null";
    
    ss << ", \"pending\": [";
    for(size_t i=0; i<pending_queue.size(); ++i) {
        ss << pending_queue[i].to_json();
        if (i < pending_queue.size()-1) ss << ",";
    }
    ss << "]";
    
    ss << ", \"history\": [";
    for(size_t i=0; i<history.size(); ++i) {
        ss << history[i].to_json();
        if (i < history.size()-1) ss << ",";
    }
    ss << "]";
    
    ss << "}";
    return ss.str();
}

// Mega-Batch 6
bool TaskManager::has_pending_tasks() {
    std::lock_guard<std::mutex> lock(mutex_);
    return !pending_queue.empty();
}
