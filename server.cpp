#include "server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iostream>

TcpServer::TcpServer(int port, std::string name) : port_(port), name_(std::move(name)) {}

TcpServer::~TcpServer() {
    stop();
}

void TcpServer::start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "[" << name_ << "] Error creating socket\n";
        return;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[" << name_ << "] Bind failed on port " << port_ << "\n";
        return;
    }

    if (listen(server_fd_, 3) < 0) {
        std::cerr << "[" << name_ << "] Listen failed\n";
        return;
    }

    running_ = true;
    acceptor_thread_ = std::thread(&TcpServer::accept_loop, this);
    std::cout << "[" << name_ << "] Started on port " << port_ << "\n";
}

void TcpServer::stop() {
    running_ = false;
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
    if (acceptor_thread_.joinable()) acceptor_thread_.join();
    
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for(int sock : client_sockets_) {
        close(sock);
    }
    client_sockets_.clear();
}

void TcpServer::broadcast(const std::string& message) {
    if (message.empty()) return;
    std::string packet = message + "\n";
    
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (int sock : client_sockets_) {
        send(sock, packet.c_str(), packet.length(), MSG_NOSIGNAL);
    }
}

void TcpServer::on_input(MessageCallback cb) {
    input_callback_ = cb;
}

void TcpServer::accept_loop() {
    while (running_) {
        sockaddr_in address;
        socklen_t addrlen = sizeof(address);
        int new_socket = accept(server_fd_, (struct sockaddr*)&address, &addrlen);
        
        if (new_socket < 0) {
            if (running_) std::cerr << "[" << name_ << "] Accept failed\n";
            continue;
        }

        std::lock_guard<std::mutex> lock(clients_mutex_);
        client_sockets_.push_back(new_socket);
        
        // Spawn a handler for this client (needed for input)
        // For output-only ports, this thread stays idle or handling basic heartbeat? 
        // We'll create a thread for every client to support input or detection of disconnect.
        std::thread t(&TcpServer::client_handler, this, new_socket);
        t.detach(); // Simplified for this prototype
    }
}

void TcpServer::client_handler(int socket_fd) {
    char buffer[1024];
    while (running_) {
        memset(buffer, 0, 1024);
        int valread = read(socket_fd, buffer, 1024);
        if (valread <= 0) {
            // Disconnected
            std::lock_guard<std::mutex> lock(clients_mutex_);
            auto it = std::find(client_sockets_.begin(), client_sockets_.end(), socket_fd);
            if (it != client_sockets_.end()) {
                client_sockets_.erase(it);
                close(socket_fd);
            }
            break;
        }
        
        // Process input
        std::string input_str(buffer, valread);
        
        // Simple HTTP Health Check Handling
        if (input_str.find("GET /health") == 0) {
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK";
            send(socket_fd, response.c_str(), response.length(), MSG_NOSIGNAL);
            
            // Close connection immediately for HTTP logic
            std::lock_guard<std::mutex> lock(clients_mutex_);
            auto it = std::find(client_sockets_.begin(), client_sockets_.end(), socket_fd);
            if (it != client_sockets_.end()) {
                client_sockets_.erase(it);
                close(socket_fd);
            }
            break;
        }

        if (input_callback_) {
            std::string msg(buffer);
            // Trim newline
            msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
            msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
            if (!msg.empty()) input_callback_(msg);
        }
    }
}
