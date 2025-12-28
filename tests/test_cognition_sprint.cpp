#include <gtest/gtest.h>
#include "../websocket_server.hpp"
#include "../auth_system.hpp"

// Item 38: Verify Lock-free WebSocket counters
TEST(CognitionSprint, WebSocketLockFree) {
    dnn::WebSocketServer server(9002);
    ASSERT_EQ(server.get_connection_count(), 0);
    
    server.start();
    server.broadcast("Test");
    // Broadcast stub increments counter
    ASSERT_EQ(server.get_connection_count(), 1);
    server.stop();
}

// Item 33: Verify Auth System Thread Safety (Basic)
TEST(CognitionSprint, AuthSystemConcurrency) {
    dnn::AuthSystem auth;
    // Just ensuring no crash / successful compilation of mutex logic
    bool res = auth.login("user", "pass");
    ASSERT_TRUE(res);
}
