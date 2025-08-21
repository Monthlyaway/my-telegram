#include "login_handler.h"
#include "../protocol/protocol_handler.h"
#include "../server/session.h"
#include "../server/session_manager.h"
#include <spdlog/spdlog.h>

LoginHandler::LoginHandler() : user_manager_(UserManager::get_instance())
{
}

bool LoginHandler::handle(const Packet &packet, std::shared_ptr<Session> session)
{
    if (!packet.has_login_request())
    {
        spdlog::error("LoginHandler received packet without login_request");
        return false;
    }

    const auto &request = packet.login_request();
    spdlog::info("Processing login request for user: {}", request.username());

    // 调用UserManager进行身份验证
    User user;
    auto result = user_manager_.authenticate_user(request.username(), request.password(), user);

    // 创建响应
    auto response_packet = ProtocolHandler::create_packet(packet.version(), packet.sequence());
    auto *response = response_packet.mutable_login_response();

    switch (result)
    {
    case UserManager::LoginResult::SUCCESS:
        response->set_success(true);
        response->set_message("Login successful");
        response->set_user_id(user.user_id);
        response->set_username(user.username);

        // 在Session中标记用户已登录
        session->set_authenticated_user(user.user_id, user.username);

        spdlog::info("User login successful: {} (ID: {})", user.username, user.user_id);
        break;

    case UserManager::LoginResult::USER_NOT_FOUND:
        response->set_success(false);
        response->set_message("User not found");
        spdlog::info("Login failed - user not found: {}", request.username());
        break;

    case UserManager::LoginResult::WRONG_PASSWORD:
        response->set_success(false);
        response->set_message("Wrong password");
        spdlog::info("Login failed - wrong password for user: {}", request.username());
        break;

    case UserManager::LoginResult::DATABASE_ERROR:
        response->set_success(false);
        response->set_message("Internal server error");
        spdlog::error("Login failed - database error for user: {}", request.username());
        break;
    }

    // 发送响应
    session->send_packet(response_packet);
    return true;
}