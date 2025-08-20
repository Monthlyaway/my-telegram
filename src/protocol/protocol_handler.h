#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <messages.pb.h>

class ProtocolHandler
{
public:
    static constexpr uint32_t PROTOCOL_VERSION = 1;
    static constexpr uint32_t MAX_FRAME_SIZE = 1024 * 1024; // 1MB max frame size

    struct Frame
    {
        uint32_t length;
        std::string data;
    };

    // Serialize a Packet to frame format [4-byte length][protobuf data]
    static std::string serialize_frame(const Packet &packet);

    // Deserialize frame data to Packet
    static bool deserialize_frame(const std::string &frame_data, Packet &packet);

    // Parse frame from binary data, returns true if complete frame found
    static bool parse_frame(const std::vector<uint8_t> &buffer, Frame &frame, size_t &consumed_bytes);

    // Create error response packet
    static Packet create_error_response(uint32_t error_code, const std::string &message, uint32_t sequence = 0);

    // Create echo response packet
    static Packet create_echo_response(const std::string &content, uint32_t sequence = 0);

    // Validate packet (version check, etc.)
    static bool validate_packet(const Packet &packet);

private:
    // Convert uint32_t to network byte order (big-endian)
    static uint32_t host_to_network(uint32_t value);

    // Convert from network byte order to host byte order
    static uint32_t network_to_host(uint32_t value);
};