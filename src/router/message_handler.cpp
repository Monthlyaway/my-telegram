#include "message_handler.h"
#include "../server/session.h"
#include "../protocol/protocol_handler.h"
#include <spdlog/spdlog.h>

/**
 * @brief EchoHandler的消息处理实现
 *
 * 处理EchoRequest消息：
 * 1. 验证消息包含EchoRequest
 * 2. 提取消息内容
 * 3. 创建EchoResponse响应
 * 4. 通过session发送响应
 */
bool EchoHandler::handle(const Packet &packet, std::shared_ptr<Session> session)
{
    if (!packet.has_echo_request())
    {
        spdlog::warn("EchoHandler received packet without echo_request");
        return false;
    }

    const auto &echo_req = packet.echo_request();
    spdlog::info("EchoHandler processing: '{}'", echo_req.content());

    // 创建Echo响应
    auto response = ProtocolHandler::create_echo_response(
        echo_req.content(),
        packet.sequence());

    // 通过session发送响应
    session->send_packet(response);

    return true;
}