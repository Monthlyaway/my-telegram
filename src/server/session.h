#pragma once

#define ASIO_STANDALONE
#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <messages.pb.h>
#include "../protocol/protocol_handler.h"

// Forward declarations
class MessageRouter;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(asio::ip::tcp::socket socket, std::shared_ptr<MessageRouter> router);
    ~Session();

    void start();
    void send_packet(const Packet &packet);

    // User authentication methods
    void set_authenticated_user(int64_t user_id, const std::string &username);
    bool is_authenticated() const;
    int64_t get_user_id() const;
    const std::string &get_username() const;

    // Allow Server to access socket for async_accept
    asio::ip::tcp::socket socket_;

private:
    void do_read();
    void do_write();
    void handle_packet(const Packet &packet);
    void process_frame_buffer();

    std::vector<uint8_t> read_buffer_;
    std::string write_buffer_;
    std::array<uint8_t, 4096> data_; // Larger buffer for binary data

    // Message router for handling packets
    std::shared_ptr<MessageRouter> message_router_;

    // User authentication state
    bool authenticated_ = false;
    int64_t user_id_ = 0;
    std::string username_;
};