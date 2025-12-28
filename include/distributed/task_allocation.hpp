#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <functional>

namespace dnn::distributed {

// Distributed Task Allocation with load balancing
class TaskAllocation {
public:
    struct Task {
        std::string id;
        std::string type;
        int priority;           // Higher = more important
        int estimated_cost;     // Processing cost estimate
        std::vector<std::string> required_capabilities;
        std::string assigned_worker;
        bool completed;
        
        Task() : priority(0), estimated_cost(1), completed(false) {}
        
        Task(const std::string& task_id, const std::string& task_type, 
             int prio = 0, int cost = 1)
            : id(task_id), type(task_type), priority(prio), 
              estimated_cost(cost), completed(false) {}
        
        bool operator<(const Task& other) const {
            return priority < other.priority;  // For priority queue (max heap)
        }
    };
    
    struct Worker {
        std::string id;
        int capacity;
        int current_load;
        std::vector<std::string> capabilities;
        bool available;
        
        Worker() : capacity(100), current_load(0), available(true) {}
        
        Worker(const std::string& worker_id, int cap = 100)
            : id(worker_id), capacity(cap), current_load(0), available(true) {}
        
        int remaining_capacity() const {
            return capacity - current_load;
        }
    };
    
    enum class AllocationStrategy {
        ROUND_ROBIN,        // Simple round-robin
        LEAST_LOADED,       // Assign to worker with lowest load
        CAPABILITY_MATCH,   // Match task requirements to worker capabilities
        PRIORITY_FIRST      // Prioritize high-priority tasks
    };
    
    TaskAllocation(AllocationStrategy strategy = AllocationStrategy::LEAST_LOADED)
        : strategy_(strategy), next_worker_idx_(0) {}
    
    // Register a worker
    void register_worker(const std::string& worker_id, int capacity = 100,
                        const std::vector<std::string>& capabilities = {}) {
        workers_[worker_id] = Worker(worker_id, capacity);
        workers_[worker_id].capabilities = capabilities;
    }
    
    // Submit a task
    void submit_task(const Task& task) {
        pending_tasks_.push(task);
    }
    
    // Submit a task with details
    void submit_task(const std::string& task_id, const std::string& type,
                    int priority = 0, int cost = 1,
                    const std::vector<std::string>& capabilities = {}) {
        Task task(task_id, type, priority, cost);
        task.required_capabilities = capabilities;
        pending_tasks_.push(task);
    }
    
    // Allocate pending tasks to workers
    std::vector<Task> allocate_tasks() {
        std::vector<Task> allocated;
        
        while (!pending_tasks_.empty()) {
            Task task = pending_tasks_.top();
            pending_tasks_.pop();
            
            std::string worker_id;
            
            switch (strategy_) {
                case AllocationStrategy::ROUND_ROBIN:
                    worker_id = allocate_round_robin();
                    break;
                case AllocationStrategy::LEAST_LOADED:
                    worker_id = allocate_least_loaded(task.estimated_cost);
                    break;
                case AllocationStrategy::CAPABILITY_MATCH:
                    worker_id = allocate_by_capability(task);
                    break;
                case AllocationStrategy::PRIORITY_FIRST:
                    worker_id = allocate_least_loaded(task.estimated_cost);
                    break;
            }
            
            if (!worker_id.empty()) {
                task.assigned_worker = worker_id;
                workers_[worker_id].current_load += task.estimated_cost;
                allocated_tasks_[task.id] = task;
                allocated.push_back(task);
            } else {
                // No available worker, put back in queue
                pending_tasks_.push(task);
                break;
            }
        }
        
        return allocated;
    }
    
    // Mark task as completed
    void complete_task(const std::string& task_id) {
        auto it = allocated_tasks_.find(task_id);
        if (it != allocated_tasks_.end()) {
            Task& task = it->second;
            task.completed = true;
            
            // Free up worker capacity
            if (!task.assigned_worker.empty() && workers_.count(task.assigned_worker)) {
                workers_[task.assigned_worker].current_load -= task.estimated_cost;
                workers_[task.assigned_worker].current_load = 
                    std::max(0, workers_[task.assigned_worker].current_load);
            }
            
            completed_tasks_.push_back(task);
            allocated_tasks_.erase(it);
        }
    }
    
    // Get worker load
    int get_worker_load(const std::string& worker_id) const {
        auto it = workers_.find(worker_id);
        return it != workers_.end() ? it->second.current_load : 0;
    }
    
    // Set worker availability
    void set_worker_available(const std::string& worker_id, bool available) {
        if (workers_.count(worker_id)) {
            workers_[worker_id].available = available;
        }
    }
    
    // Get statistics
    struct Stats {
        int pending_tasks;
        int allocated_tasks;
        int completed_tasks;
        int total_workers;
        int available_workers;
        float avg_worker_load;
    };
    
    Stats get_stats() const {
        Stats stats{};
        stats.pending_tasks = pending_tasks_.size();
        stats.allocated_tasks = allocated_tasks_.size();
        stats.completed_tasks = completed_tasks_.size();
        stats.total_workers = workers_.size();
        
        int total_load = 0;
        for (const auto& [id, worker] : workers_) {
            if (worker.available) {
                stats.available_workers++;
            }
            total_load += worker.current_load;
        }
        
        if (!workers_.empty()) {
            stats.avg_worker_load = static_cast<float>(total_load) / workers_.size();
        }
        
        return stats;
    }
    
    // Get tasks assigned to a worker
    std::vector<Task> get_worker_tasks(const std::string& worker_id) const {
        std::vector<Task> tasks;
        for (const auto& [id, task] : allocated_tasks_) {
            if (task.assigned_worker == worker_id) {
                tasks.push_back(task);
            }
        }
        return tasks;
    }
    
private:
    AllocationStrategy strategy_;
    std::priority_queue<Task> pending_tasks_;
    std::unordered_map<std::string, Task> allocated_tasks_;
    std::vector<Task> completed_tasks_;
    std::unordered_map<std::string, Worker> workers_;
    size_t next_worker_idx_;
    
    std::string allocate_round_robin() {
        if (workers_.empty()) return "";
        
        std::vector<std::string> worker_ids;
        for (const auto& [id, _] : workers_) {
            worker_ids.push_back(id);
        }
        
        size_t start_idx = next_worker_idx_;
        do {
            const std::string& worker_id = worker_ids[next_worker_idx_];
            next_worker_idx_ = (next_worker_idx_ + 1) % worker_ids.size();
            
            if (workers_.at(worker_id).available) {
                return worker_id;
            }
        } while (next_worker_idx_ != start_idx);
        
        return "";
    }
    
    std::string allocate_least_loaded(int task_cost) {
        std::string best_worker;
        int max_remaining = -1;
        
        for (const auto& [id, worker] : workers_) {
            if (worker.available && worker.remaining_capacity() >= task_cost) {
                if (worker.remaining_capacity() > max_remaining) {
                    max_remaining = worker.remaining_capacity();
                    best_worker = id;
                }
            }
        }
        
        return best_worker;
    }
    
    std::string allocate_by_capability(const Task& task) {
        std::string best_worker;
        int max_remaining = -1;
        
        for (const auto& [id, worker] : workers_) {
            if (!worker.available || worker.remaining_capacity() < task.estimated_cost) {
                continue;
            }
            
            // Check if worker has required capabilities
            bool has_capabilities = true;
            for (const auto& required : task.required_capabilities) {
                if (std::find(worker.capabilities.begin(), worker.capabilities.end(), required)
                    == worker.capabilities.end()) {
                    has_capabilities = false;
                    break;
                }
            }
            
            if (has_capabilities && worker.remaining_capacity() > max_remaining) {
                max_remaining = worker.remaining_capacity();
                best_worker = id;
            }
        }
        
        return best_worker;
    }
};

} // namespace dnn::distributed
