#pragma once

#define ASIO_STANDALONE
#include <asio.hpp>
#include <memory>
#include <thread>
#include <vector>
#include "session.h"
#include "../config/config.h"

// Forward declarations
class MessageRouter;

class Server {
public:
    Server(const Config& config);
    ~Server();

    bool start();
    void stop();

private:
    void do_accept();
    void run_worker_threads();
    void initialize_message_router();

    const Config& config_;
    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> worker_threads_;
    bool running_;
    
    // Message routing system
    std::shared_ptr<MessageRouter> message_router_;
};