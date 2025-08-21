#include "server.h"
#include "session_manager.h"
#include "../router/message_router.h"
#include "../router/message_handler.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include <algorithm>

/**
 * Server通过config类创建
 * 初始化event loop负责监听和dispatch
 * 初始化acceptor负责创建新的socket
 * 初始化running flag为false
 */
Server::Server(const Config &config)
    : config_(config), io_context_(), acceptor_(io_context_), running_(false), message_router_(nullptr)
{
    spdlog::info("=== Server Constructor ===");
    spdlog::info("Calling initialize_message_router()...");
    initialize_message_router();

    if (message_router_)
    {
        spdlog::info("Server constructor completed successfully with MessageRouter");
    }
    else
    {
        spdlog::error("Server constructor completed but MessageRouter is null!");
    }
}

/**
 * Server的析构函数用stop函数封装
 */
Server::~Server()
{
    stop();
}

/**
 * 启动Server
 * 1. 读取server config
 * 2. 把acceptor和端口绑定起来，然后设置为监听端口
 * 3. do_accept
 * 4. 根据线程数run_worker_threads
 * acceptor在io_context上异步accept->产生socket绑定到同一个io_context->Session使用改socket发起异步读写，所有回调函数由io_context.run()执行
 *
 * acceptor所创建的所有连接socket都注册到了io_context中
 * 简称：server start->acceptor bind and listen->do_accept创建socket注册回调函数->io_context.run()执行回调函数
 */
bool Server::start()
{
    try
    {
        const auto &server_config = config_.get_server_config();

        // Setup acceptor
        asio::ip::tcp::endpoint endpoint(
            asio::ip::address::from_string(server_config.host),
            server_config.port);

        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();

        spdlog::info("Server started on {}:{}",
                     server_config.host, server_config.port);
        spdlog::info("Max connections: {}, Worker threads: {}",
                     server_config.max_connections, server_config.worker_threads);

        running_ = true;
        do_accept();
        run_worker_threads();

        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Failed to start server: {}", e.what());
        return false;
    }
}

void Server::stop()
{
    if (running_)
    {
        spdlog::info("Stopping server...");
        running_ = false;

        acceptor_.close();
        io_context_.stop();

        // Wait for all worker threads to finish
        for (auto &thread : worker_threads_)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        worker_threads_.clear();

        spdlog::info("Server stopped");
    }
}

/**
 * 接收新的链接
 * 1. 创建一个Session对象，其内部的socket和某个Eventloop绑定
 * 2. 异步调用async_accept，立即返回不会阻塞，如果有新的连接就会调用callback 函数
 * 3. 接受的连接填入new_session->socket_中，回调函数由new_session->start封装。异步循环的意思是，如果没有新的连接，就递归调用自己
 * 简称do_accept->session start->异步读写->do_accept
 */
void Server::do_accept()
{
    auto new_session = std::make_shared<Session>(
        asio::ip::tcp::socket(io_context_), message_router_);

    acceptor_.async_accept(
        new_session->socket_,
        [this, new_session](std::error_code ec)
        {
            if (!ec)
            {
                new_session->start();
            }
            else
            {
                spdlog::error("Accept failed: {}", ec.message());
            }

            if (running_)
            {
                do_accept(); // Continue accepting new connections
            }
        });
}

/**
 * 核心函数io_context_.run(),回调函数是由io_context执行的
 * 创建多个worker threads可以并发执行io_context_.run()
 */
void Server::run_worker_threads()
{
    const auto &server_config = config_.get_server_config();

    // Create worker threads
    for (int i = 0; i < server_config.worker_threads; ++i)
    {
        worker_threads_.emplace_back([this]()
                                     {
            try {
                io_context_.run();
            } catch (const std::exception& e) {
                spdlog::error("Worker thread error: {}", e.what());
            } });
    }

    spdlog::info("Started {} worker threads", server_config.worker_threads);

    // Wait for all worker threads
    for (auto &thread : worker_threads_)
    {
        thread.join();
    }
}

/**
 * @brief 初始化Server的message router
 * 
 * 构造函数中先设为nullptr
 * 
 * 流程：make shared MessageRouter -> 注册默认的Handler
 * 
 * 如果失败，调用shared_ptr.reset()把message_router_设为nullptr,让这个 shared_ptr 放弃当前所管理的对象。
 * 
 */
void Server::initialize_message_router()
{
    spdlog::info("=== Initializing MessageRouter ===");

    try
    {
        // 创建消息路由器
        spdlog::info("Creating MessageRouter instance...");
        message_router_ = std::make_shared<MessageRouter>();

        if (!message_router_)
        {
            spdlog::error("Failed to create MessageRouter instance");
            return;
        }
        spdlog::info("MessageRouter instance created successfully");

        // 注册默认处理器
        spdlog::info("Creating EchoHandler instance...");
        auto echo_handler = std::make_shared<EchoHandler>();

        if (!echo_handler)
        {
            spdlog::error("Failed to create EchoHandler instance");
            return;
        }
        spdlog::info("EchoHandler instance created successfully");

        spdlog::info("Registering EchoHandler with MessageRouter...");
        message_router_->register_handler(MessageRouter::MessageType::ECHO_REQUEST, echo_handler);

        spdlog::info("MessageRouter initialized successfully with {} handlers",
                     message_router_->get_handler_count());
        spdlog::info("=== MessageRouter initialization complete ===");
    }
    catch (const std::exception &e)
    {
        spdlog::error("Exception during MessageRouter initialization: {}", e.what());
        message_router_.reset(); // 确保为null以便错误处理
    }
}