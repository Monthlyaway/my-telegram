#pragma once

#define ASIO_STANDALONE
#include <asio.hpp>
#include <memory>
#include <thread>
#include <vector>
#include "session.h"
#include "../config/config.h"

class Server {
public:
    Server(const Config& config);
    ~Server();

    bool start();
    void stop();

private:
    void do_accept();
    void run_worker_threads();

    const Config& config_;
    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> worker_threads_;
    bool running_;
};