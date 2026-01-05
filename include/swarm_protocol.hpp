#pragma once
#include <string>
#include <vector>

namespace dnn {

    struct SwarmPacket {
        std::string sender_id;
        std::string target_id; // "ALL" or specific ID
        std::string protocol_version = "1.0";
        std::string payload_type; // "MEMORY_SHARE", "ALERT", "TASK_REQUEST"
        std::string payload;
        long timestamp;
    };

    // Interface for P2P comms (stub)
    class SwarmInterface {
    public:
        virtual void broadcast(const SwarmPacket& packet) = 0;
        virtual std::vector<SwarmPacket> receive() = 0;
        virtual ~SwarmInterface() = default;
    };

} // namespace dnn
