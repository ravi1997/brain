#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <netinet/in.h>

class TcpServer {
public:
    using MessageCallback = std::function<void(const std::string&)>;

    TcpServer(int port, std::string name);
    ~TcpServer();

    void start();
    void stop();
    
    // Send to all connected clients
    void broadcast(const std::string& message);
    
    // Set callback for received messages
    void on_input(MessageCallback cb);

    int get_port() const { return port_; }
    std::string get_name() const { return name_; }

private:
    void accept_loop();
    void client_handler(int socket_fd);
    void cleanup_stale_clients();

    int port_;
    std::string name_;
    int server_fd_{-1};
    std::atomic<bool> running_{false};
    
    std::thread acceptor_thread_;
    std::vector<std::thread> client_threads_;
    
    // Active client sockets
    std::vector<int> client_sockets_;
    std::mutex clients_mutex_;
    
    MessageCallback input_callback_;
};
