#include <gtest/gtest.h>
#include "../rate_limiter.hpp"
#include <thread>
#include <chrono>

TEST(RateLimiterTest, BasicLimit) {
    // 5 tokens, refill 1 per second
    RateLimiter limiter(5, 1);
    
    // Should allow 5 immediately
    EXPECT_TRUE(limiter.allow("user1"));
    EXPECT_TRUE(limiter.allow("user1"));
    EXPECT_TRUE(limiter.allow("user1"));
    EXPECT_TRUE(limiter.allow("user1"));
    EXPECT_TRUE(limiter.allow("user1"));
    
    // 6th should fail
    EXPECT_FALSE(limiter.allow("user1"));
}

TEST(RateLimiterTest, Refill) {
    // 1 token, refill 10 per second
    RateLimiter limiter(1, 10);
    
    EXPECT_TRUE(limiter.allow("user2"));
    EXPECT_FALSE(limiter.allow("user2"));
    
    // Wait for refill (100ms should be enough for 1 token at 10/s)
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_TRUE(limiter.allow("user2"));
}

TEST(RateLimiterTest, IndependentUsers) {
    RateLimiter limiter(1, 1);
    
    EXPECT_TRUE(limiter.allow("userA"));
    EXPECT_FALSE(limiter.allow("userA"));
    
    // User B should be fresh
    EXPECT_TRUE(limiter.allow("userB"));
}
