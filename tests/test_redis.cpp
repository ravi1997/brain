#include <gtest/gtest.h>
#include "redis_client.hpp"
#include <thread>
#include <chrono>

TEST(RedisTest, ConnectAndSetGet) {
    RedisClient client("redis", 6379);
    
    // Retry connection for up to 5 seconds
    bool connected = false;
    for (int i = 0; i < 50; ++i) {
        if (client.connect()) {
            connected = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!connected) {
        std::cout << "[WARN] Redis unreachable after 5 seconds. Skipping test." << std::endl;
        return;
    }

    client.set("test_key", "hello_world", 10);
    auto val = client.get("test_key");
    
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), "hello_world");
    
    auto missing = client.get("non_existent_key");
    EXPECT_FALSE(missing.has_value());
}
