#pragma once
#include <hiredis/hiredis.h>
#include <string>
#include <iostream>
#include <mutex>
#include <optional>

class RedisClient {
public:
    RedisClient(const std::string& host = "redis", int port = 6379) 
        : host_(host), port_(port), context_(nullptr) {}

    ~RedisClient() {
        if (context_) {
            redisFree(context_);
        }
    }

    bool connect() {
        if (context_) return true;
        
        struct timeval timeout = { 1, 500000 }; // 1.5 seconds
        context_ = redisConnectWithTimeout(host_.c_str(), port_, timeout);
        
        if (context_ == NULL || context_->err) {
            if (context_) {
                std::cerr << "Redis Connection Error: " << context_->errstr << std::endl;
                redisFree(context_);
                context_ = nullptr;
            } else {
                std::cerr << "Connection error: can't allocate redis context" << std::endl;
            }
            return false;
        }
        std::cout << "Connected to Redis at " << host_ << ":" << port_ << std::endl;
        return true;
    }

    void set(const std::string& key, const std::string& value, int ttl_seconds = 60) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!connect()) return;

        redisReply* reply = (redisReply*)redisCommand(context_, "SETEX %s %d %s", key.c_str(), ttl_seconds, value.c_str());
        if (reply) freeReplyObject(reply);
    }

    std::optional<std::string> get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!connect()) return std::nullopt;

        redisReply* reply = (redisReply*)redisCommand(context_, "GET %s", key.c_str());
        std::optional<std::string> result;
        
        if (reply) {
            if (reply->type == REDIS_REPLY_STRING) {
                result = std::string(reply->str, reply->len);
            }
            freeReplyObject(reply);
        }
        return result;
    }

private:
    std::string host_;
    int port_;
    redisContext* context_;
    std::mutex mutex_;
};
