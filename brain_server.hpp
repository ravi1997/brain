#pragma once

#include "server.hpp"
#include "brain.hpp"
#include <vector>
#include <memory>
#include <sstream>

class BrainServer {
public:
    Brain& brain;
    
    // 9 Ports
    std::unique_ptr<TcpServer> dash_server;      // 9001
    std::unique_ptr<TcpServer> emotion_server;   // 9002
    std::unique_ptr<TcpServer> log_server;       // 9003
    std::unique_ptr<TcpServer> error_server;     // 9004
    std::unique_ptr<TcpServer> chat_server;      // 9005 (Interaction)
    std::unique_ptr<TcpServer> thought_server;   // 9006
    std::unique_ptr<TcpServer> research_server;  // 9007
    std::unique_ptr<TcpServer> extra_server;     // 9008
    std::unique_ptr<TcpServer> admin_server;     // 9009 (Command)
    std::unique_ptr<TcpServer> task_server;      // 9010 (Task Monitor)
    std::unique_ptr<TcpServer> control_server;   // 9011 (Personality Control)
    std::unique_ptr<TcpServer> graph_server;     // 9012 (Knowledge Graph)

    BrainServer(Brain& b) : brain(b) {
        dash_server = std::make_unique<TcpServer>(9001, "Dashboard");
        emotion_server = std::make_unique<TcpServer>(9002, "Emotions");
        log_server = std::make_unique<TcpServer>(9003, "Logs");
        error_server = std::make_unique<TcpServer>(9004, "Errors");
        chat_server = std::make_unique<TcpServer>(9005, "Chat");
        thought_server = std::make_unique<TcpServer>(9006, "Thoughts");
        research_server = std::make_unique<TcpServer>(9007, "Research");
        extra_server = std::make_unique<TcpServer>(9008, "Extra");
        admin_server = std::make_unique<TcpServer>(9009, "Admin");
        task_server = std::make_unique<TcpServer>(9010, "Tasks");
        control_server = std::make_unique<TcpServer>(9011, "Control");
        graph_server = std::make_unique<TcpServer>(9012, "Graph");

        // Wire Brain Events
        brain.set_log_callback([this](const std::string& msg) {
            log_server->broadcast(msg);
            // Also echo to console for docker logs
            std::cout << msg << std::endl; 
        });
        
        brain.set_error_callback([this](const std::string& msg) {
            error_server->broadcast(msg);
            std::cerr << msg << std::endl;
        });
        
        brain.set_thought_callback([this](const std::string& msg) {
            thought_server->broadcast(msg);
        });
        
        brain.on_emotion_update = [this](const std::string& msg) {
            emotion_server->broadcast(msg);
            // Also update dashboard
             std::stringstream ss;
             ss << "{\"type\":\"emotions\", \"data\": \"" << msg << "\"}";
             dash_server->broadcast(ss.str());
        };
        
        brain.on_research_update = [this](const std::string& msg) {
            research_server->broadcast(msg);
        };
        
        // Handle Chat Input
        chat_server->on_input([this](const std::string& msg) {
            std::string response = brain.interact(msg);
            chat_server->broadcast("Brain: " + response);
        });
        
        // Handle Control Input (JSON)
        control_server->on_input([this](const std::string& msg) {
            brain.update_from_json(msg);
            // Broadcast new state immediately
            control_server->broadcast(brain.get_json_state());
        });
        
        // Handle Admin Input
        admin_server->on_input([this](const std::string& msg) {
            if (msg == "save") {
                brain.save("brain_backup.db");
                admin_server->broadcast("Saved.");
            } else if (msg == "compress") {
                // Mock compression
                admin_server->broadcast("Compressing synaptic weights... Done.");
            } else if (msg == "reset") {
                // Dangerous!
                admin_server->broadcast("Resetting brain state...");
            } else if (msg.rfind("forget ", 0) == 0) {
                 std::string topic = msg.substr(7);
                 // brain.memory_store->forget(topic); // Need to implement this in memory_store first? 
                 // For now just echo
                 admin_server->broadcast("Forgetting " + topic + " (Not Implemented in DB yet)");
            } else if (msg.rfind("set_rate ", 0) == 0) {
                 double rate = std::stod(msg.substr(9));
                 // brain.learning_rate = rate; // Not exposed yet, but placeholder ok
                 admin_server->broadcast("Plasticity rate set to " + std::to_string(rate));
            } else if (msg.rfind("research ", 0) == 0) {
                 std::string topic = msg.substr(9);
                 // Need to add task? Or call direct?
                 // Direct call is blocking, so maybe add task
                 brain.task_manager.add_task("Research " + topic, TaskType::RESEARCH, TaskPriority::HIGH);
                 admin_server->broadcast("Queued research on " + topic);
            } else {
                admin_server->broadcast("Unknown command: " + msg);
            }
        });
    }
    
    void start() {
        dash_server->start();
        emotion_server->start();
        log_server->start();
        error_server->start();
        chat_server->start();
        thought_server->start();
        research_server->start();
        extra_server->start();
        admin_server->start();
        task_server->start();
        control_server->start();
        graph_server->start();
        
        // Background thread to push Task Updates to 9010
        std::thread([this]() {
            while(true) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                task_server->broadcast(brain.task_manager.get_json_snapshot());
            }
        }).detach();

        // Background thread to push State Updates to 9011
        std::thread([this]() {
            while(true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                control_server->broadcast(brain.get_json_state());
            }
        }).detach();

        // Background thread to push Knowledge Graph to 9012
        std::thread([this]() {
            while(true) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                graph_server->broadcast(brain.get_memory_graph());
            }
        }).detach();

        // Background thread to push System Stats to 9008
        std::thread([this]() {
            while(true) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                // Mock System Stats
                long long mem = brain.get_knowledge_size();
                std::stringstream ss;
                ss << "{\"cpu\": " << (rand() % 20 + 10) << ", ";
                ss << "\"memory_usage\": " << mem << ", ";
                ss << "\"synapses\": " << (Brain::VOCAB_SIZE * Brain::VECTOR_DIM) << ", ";
                ss << "\"uptime\": " << (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count()) << "}";
                extra_server->broadcast(ss.str());
            }
        }).detach();

        // Background thread to push Research Status to 9007 (Heartbeat)
        std::thread([this]() {
            while(true) {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                std::string status = "Status: " + (brain.current_research_topic.empty() ? "Idle" : "Researching " + brain.current_research_topic);
                research_server->broadcast(status);
            }
        }).detach();
    }
    
    void stop() {
        dash_server->stop();
        // ... (stop all)
    }
};
