#include "message_router.h"
#include "../server/session.h"
#include "../protocol/protocol_handler.h"
#include <spdlog/spdlog.h>

MessageRouter::MessageRouter()
{
    spdlog::info("MessageRouter initialized");
}

void MessageRouter::register_handler(MessageType type, std::shared_ptr<MessageHandler> handler)
{
    if (!handler)
    {
        spdlog::error("Cannot register null handler for type {}", message_type_to_string(type));
        return;
    }

    handlers_[type] = handler;
    spdlog::info("Registered handler '{}' for message type {}",
                 handler->get_handler_name(), message_type_to_string(type));
}

bool MessageRouter::route_message(const Packet &packet, std::shared_ptr<Session> session)
{
    if (!session)
    {
        spdlog::error("Cannot route message with null session");
        return false;
    }

    // 确定消息类型
    MessageType type = determine_message_type(packet);
    spdlog::debug("Routing message type: {}", message_type_to_string(type));

    // 查找对应的处理器, it指向一个pair：std::pair<const MessageType, std::shared_ptr<MessageHandler>>
    auto it = handlers_.find(type);
    if (it == handlers_.end())
    {
        spdlog::warn("No handler found for message type: {}", message_type_to_string(type));

        // 发送错误响应
        send_error_response(3001,
                            "Unsupported message type: " + message_type_to_string(type),
                            packet.sequence(),
                            session);
        return false;
    }

    // 调用处理器处理消息
    try
    {
        bool result = it->second->handle(packet, session);
        if (result)
        {
            spdlog::debug("Message handled successfully by {}", it->second->get_handler_name());
        }
        else
        {
            spdlog::warn("Handler {} failed to process message", it->second->get_handler_name());
        }
        return result;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Exception in handler {}: {}", it->second->get_handler_name(), e.what());

        // 发送错误响应
        send_error_response(3002,
                            "Internal handler error: " + std::string(e.what()),
                            packet.sequence(),
                            session);
        return false;
    }
}

MessageRouter::MessageType MessageRouter::determine_message_type(const Packet &packet) const
{
    // 根据oneof字段确定消息类型
    if (packet.has_echo_request())
    {
        return MessageType::ECHO_REQUEST;
    }

    // 未来可以添加更多消息类型检查
    // if (packet.has_register_request()) {
    //     return MessageType::USER_REGISTER;
    // }
    // if (packet.has_login_request()) {
    //     return MessageType::USER_LOGIN;
    // }

    return MessageType::UNKNOWN;
}

void MessageRouter::send_error_response(uint32_t error_code, const std::string &message,
                                        uint32_t sequence, std::shared_ptr<Session> session)
{
    try
    {
        auto error_packet = ProtocolHandler::create_error_response(error_code, message, sequence);
        session->send_packet(error_packet);
        spdlog::debug("Sent error response: code={}, message='{}'", error_code, message);
    }
    catch (const std::exception &e)
    {
        spdlog::error("Failed to send error response: {}", e.what());
    }
}

std::string MessageRouter::message_type_to_string(MessageType type) const
{
    switch (type)
    {
    case MessageType::ECHO_REQUEST:
        return "ECHO_REQUEST";
    case MessageType::UNKNOWN:
        return "UNKNOWN";
    default:
        return "UNDEFINED";
    }
}