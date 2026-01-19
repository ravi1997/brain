#include "server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iostream>

TcpServer::TcpServer(int port, std::string name) 
    : port_(port), name_(std::move(name)), rate_limiter_(50, 1) {}

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

    if (listen(server_fd_, 128) < 0) {
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
    authenticated_sockets_.clear();
}

void TcpServer::broadcast(const std::string& message) {
    if (message.empty()) return;
    std::string packet = message + "\n";
    
    std::vector<int> targets;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        if (token_.empty()) {
            targets = client_sockets_;
        } else {
            targets = authenticated_sockets_;
        }
    }

    for (int sock : targets) {
        // Use MSG_DONTWAIT to avoid blocking the whole server if one client is slow
        ssize_t sent = send(sock, packet.c_str(), packet.length(), MSG_NOSIGNAL | MSG_DONTWAIT);
        if (sent < 0 && (errno == EPIPE || errno == EBADF)) {
            // Client likely disconnected, will be cleaned up by handler
        }
    }
}

void TcpServer::on_input(MessageCallback cb) {
    input_callback_ = cb;
}

void TcpServer::set_token(const std::string& token) {
    token_ = token;
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

        // Rate Limit Check
        if (!check_rate_limit(address.sin_addr.s_addr)) {
            std::string response = "HTTP/1.1 429 Too Many Requests\r\nConnection: close\r\n\r\nToo Many Requests";
            send(new_socket, response.c_str(), response.length(), MSG_NOSIGNAL);
            close(new_socket);
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
    bool authenticated = token_.empty();

    while (running_) {
        memset(buffer, 0, 1024);
        int valread = read(socket_fd, buffer, 1024);
        if (valread <= 0) {
            // Disconnected
            std::lock_guard<std::mutex> lock(clients_mutex_);
            auto it = std::find(client_sockets_.begin(), client_sockets_.end(), socket_fd);
            if (it != client_sockets_.end()) {
                client_sockets_.erase(it);
                auto auth_it = std::find(authenticated_sockets_.begin(), authenticated_sockets_.end(), socket_fd);
                if (auth_it != authenticated_sockets_.end()) {
                    authenticated_sockets_.erase(auth_it);
                }
                close(socket_fd);
            }
            break;
        }
        
        // Process input
        std::string input_str(buffer, valread);
        
        // Simple HTTP Health Check Handling - ALWAYS ALLOWED for k8s/docker health
        if (input_str.find("GET /health") == 0) {
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOK";
            send(socket_fd, response.c_str(), response.length(), MSG_NOSIGNAL);
            
            // Close connection immediately for HTTP logic
            std::lock_guard<std::mutex> lock(clients_mutex_);
            auto it = std::find(client_sockets_.begin(), client_sockets_.end(), socket_fd);
            if (it != client_sockets_.end()) {
                client_sockets_.erase(it);
                auto auth_it = std::find(authenticated_sockets_.begin(), authenticated_sockets_.end(), socket_fd);
                if (auth_it != authenticated_sockets_.end()) {
                    authenticated_sockets_.erase(auth_it);
                }
                close(socket_fd);
            }
            break;
        }

        if (!authenticated) {
            // Expect "AUTH <token>\n"
            if (input_str.find("AUTH ") == 0) {
                std::string received_token = input_str.substr(5);
                // Trim whitespace/newlines
                received_token.erase(std::remove(received_token.begin(), received_token.end(), '\n'), received_token.end());
                received_token.erase(std::remove(received_token.begin(), received_token.end(), '\r'), received_token.end());
                
                if (received_token == token_) {
                    authenticated = true;
                    {
                        std::lock_guard<std::mutex> lock(clients_mutex_);
                        authenticated_sockets_.push_back(socket_fd);
                    }
                    std::string success = "AUTH_OK\n";
                    send(socket_fd, success.c_str(), success.length(), MSG_NOSIGNAL);
                    continue; // Wait for next message
                }
            }
            
            // Authentication failed
            std::string failure = "AUTH_FAILED\n";
            send(socket_fd, failure.c_str(), failure.length(), MSG_NOSIGNAL);
            
            std::lock_guard<std::mutex> lock(clients_mutex_);
            auto it = std::find(client_sockets_.begin(), client_sockets_.end(), socket_fd);
            if (it != client_sockets_.end()) {
                client_sockets_.erase(it);
                auto auth_it = std::find(authenticated_sockets_.begin(), authenticated_sockets_.end(), socket_fd);
                if (auth_it != authenticated_sockets_.end()) {
                    authenticated_sockets_.erase(auth_it);
                }
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

bool TcpServer::check_rate_limit(uint32_t ip) {
    return rate_limiter_.check_limit();
}
