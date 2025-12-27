#pragma once
#include <chrono>
#include <mutex>
#include <unordered_map>

class RateLimiter {
public:
    RateLimiter(size_t max_tokens, size_t refill_rate)
        : max_tokens_(max_tokens), refill_rate_(refill_rate) {}

    bool allow(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        
        if (buckets_.find(client_id) == buckets_.end()) {
            buckets_[client_id] = {max_tokens_, now};
        }
        auto& bucket = buckets_[client_id];
        
        // Refill
        long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - bucket.last_refill).count();
        size_t tokens_to_add = (elapsed_ms * refill_rate_) / 1000;
        
        if (tokens_to_add > 0) {
            bucket.tokens = std::min(max_tokens_, bucket.tokens + tokens_to_add);
            bucket.last_refill = now;
        }

        // Consume
        if (bucket.tokens >= 1) {
            bucket.tokens -= 1;
            return true;
        }
        
        return false;
    }

private:
    struct Bucket {
        size_t tokens;
        std::chrono::steady_clock::time_point last_refill;
    };

    size_t max_tokens_;
    size_t refill_rate_; // tokens per second
    std::unordered_map<std::string, Bucket> buckets_;
    std::mutex mutex_;
};
