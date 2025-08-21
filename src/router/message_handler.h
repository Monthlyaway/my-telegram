#pragma once

#include <memory>
#include <messages.pb.h>

// Forward declaration
class Session;

/**
 * @brief 消息处理器接口
 *
 * 定义所有消息处理器的统一接口，用于处理特定类型的消息。
 * 所有具体的消息处理器都应继承此接口并实现handle方法。
 */
class MessageHandler
{
public:
    virtual ~MessageHandler() = default;

    /**
     * @brief 处理消息
     * @param packet 要处理的protobuf消息包
     * @param session 发送消息的会话，用于发送响应
     * @return true 如果消息处理成功，false 如果处理失败
     */
    virtual bool handle(const Packet &packet, std::shared_ptr<Session> session) = 0;

    /**
     * @brief 获取处理器名称，用于日志和调试
     * @return 处理器名称
     */
    virtual std::string get_handler_name() const = 0;
};

/**
 * @brief Echo消息处理器
 *
 * 处理EchoRequest消息，返回EchoResponse
 */
class EchoHandler : public MessageHandler
{
public:
    bool handle(const Packet &packet, std::shared_ptr<Session> session) override;
    std::string get_handler_name() const override { return "EchoHandler"; }
};