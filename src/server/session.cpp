#include "session.h"
#include <iostream>
#include <algorithm>

/**
 * Session持有socker并封装连接socket上所有事件的异步读写逻辑（callback函数）
 * 拥有一个socket_对象，封装对单个客户端的异步读写
 */
Session::Session(asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
{
}

Session::~Session()
{
    spdlog::info("Session destroyed for client: {}",
                 socket_.remote_endpoint().address().to_string());
}

/**
 * 当新的连接被accept的时候，启动Session，new client created
 * remote_endpoint是客户端的地址
 * 调用do_read开始异步读
 */
void Session::start()
{
    auto remote_endpoint = socket_.remote_endpoint();
    spdlog::info("New client connected: {}:{}",
                 remote_endpoint.address().to_string(),
                 remote_endpoint.port());
    do_read();
}

/**
 * 流程：创建一个自己的副本传到lambda中->调用async_read_some->读到的数据在buffer中->handle_message(message)->把Echo写到write_buffer中等待do_write的时候发出去
 * 简称：do_read->handle_message->do_write->do_read
 * shared_from_this生成一个std::shared_ptr<Session>并持有当前对象的共享所有权，然后在后面的异步回调里把 self 捕获（按值），以保证在回调执行期间 Session 对象不会被销毁。它是防止异步回调中使用悬空 this 导致未定义行为的常见手法。
 * 异步操作（像 socket_.async_read_some(...)）的回调会在未来某个时刻由 io_context 调用（也即注册）若在这期间 Session 被外部释放（shared_ptr 引用计数降为 0），this 会变成悬空指针，回调里访问成员会产生未定义行为。
 * 通过在进入异步调用前做 auto self = shared_from_this();，并把 self 捕获到 lambda 中，能延长 Session 的生存期直到 lambda 完成，从而避免悬空。
 * ession 必须继承自 std::enable_shared_from_this<Session>，且该 Session 对象必须是由 std::shared_ptr 管理（例如 std::make_shared<Session>(...)）。
 * async_read_some(asio::buffer对象，我在 data_ 地址处有 data_.size() 字节的可写空间，请把读到的数据写到这里)
 * handler 会通过 socket 的 executor 调度（通常是 io_context 的 executor）。因此，执行 handler 的是调用 io_context.run() 的线程之一
 * 因此，若你没有调用任何 io_context.run()，异步操作完成后 handler 不会被执行
 */
void Session::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(
        asio::buffer(data_, data_.size()),
        [this, self](std::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::string message(data_.data(), length);

                // Remove newline characters
                message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
                message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

                spdlog::info("Received message: '{}'", message);
                handle_message(message);
            }
            else
            {
                spdlog::info("Client disconnected: {}", ec.message());
            }
        });
}

/**
 * 流程：创建Session的副本->调用async_write->发出buffer中的数据->继续异步读开始下一轮（不然只有一轮）
 * 简称：do_write->do_read
 * start->do_read->do_write->do_read
 * 封装异步写，和do_read类似，需要一块写Buffer
 * 第一个参数是socket，因为是写到一个指定的socket连接上
 */
void Session::do_write()
{
    auto self(shared_from_this());
    asio::async_write(
        socket_,
        asio::buffer(write_buffer_),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                write_buffer_.clear();
                do_read(); // Continue reading after write
            }
            else
            {
                spdlog::error("Write failed: {}", ec.message());
            }
        });
}

/**
 * handle Echo message：把message写到write_buffer中->do_write
 */
void Session::handle_message(const std::string &message)
{
    // Echo the message back to client
    write_buffer_ = "Echo: " + message + "\n";
    spdlog::info("Echoing back: '{}'", message);
    do_write();
}