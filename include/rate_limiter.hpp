#pragma once
#include <chrono>
#include <mutex>

namespace dnn {

class RateLimiter {
public:
    RateLimiter(int max_tokens, int refill_rate) 
        : max_tokens_(max_tokens), refill_rate_(refill_rate), tokens_(max_tokens) {
        last_refill_ = std::chrono::steady_clock::now();
    }

    bool check_limit(int cost = 1) {
        std::lock_guard<std::mutex> lock(mu_);
        refill();
        if (tokens_ >= cost) {
            tokens_ -= cost;
            return true;
        }
        return false;
    }

    void optimize_latency() {
        // Feature 163 stub
    }

private:
    void refill() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_refill_).count();
        if (elapsed > 0) {
            tokens_ = std::min(max_tokens_, tokens_ + static_cast<int>(elapsed * refill_rate_));
            last_refill_ = now;
        }
    }

    int max_tokens_;
    int refill_rate_;
    int tokens_;
    std::chrono::steady_clock::time_point last_refill_;
    std::mutex mu_;
};

} // namespace dnn
