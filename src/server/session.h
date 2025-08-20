#pragma once

#define ASIO_STANDALONE
#include <asio.hpp>
#include <memory>
#include <string>
#include <spdlog/spdlog.h>

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(asio::ip::tcp::socket socket);
    ~Session();

    void start();

    // Allow Server to access socket for async_accept
    asio::ip::tcp::socket socket_;

private:
    void do_read();
    void do_write();
    void handle_message(const std::string& message);
    std::string read_buffer_;
    std::string write_buffer_;
    std::array<char, 1024> data_;
};