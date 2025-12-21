#pragma once
#include <string>
#include <vector>
#include <deque>

struct PlannedTask {
    std::string description;
    int priority; // 0-10
    bool completed = false;
};

class PlanningUnit {
public:
    std::deque<PlannedTask> task_queue;

    void add_task(const std::string& desc, int priority) {
        task_queue.push_back({desc, priority, false});
        // Sort by priority (descending)
        std::sort(task_queue.begin(), task_queue.end(), [](const PlannedTask& a, const PlannedTask& b) {
            return a.priority > b.priority;
        });
    }

    PlannedTask get_next_task() {
        if (task_queue.empty()) return {"", 0, true};
        PlannedTask t = task_queue.front();
        task_queue.pop_front();
        return t;
    }

    void decompose_goal(const std::string& goal) {
        // Mock decomposition
        add_task("Analyze requirements for " + goal, 5);
        add_task("Execute " + goal, 4);
        add_task("Verify " + goal, 3);
    }
};
