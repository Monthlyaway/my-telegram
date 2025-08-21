#include "register_handler.h"
#include "../protocol/protocol_handler.h"
#include "../server/session.h"
#include <spdlog/spdlog.h>

RegisterHandler::RegisterHandler() : user_manager_(UserManager::get_instance())
{
}

bool RegisterHandler::handle(const Packet &packet, std::shared_ptr<Session> session)
{
    if (!packet.has_register_request())
    {
        spdlog::error("RegisterHandler received packet without register_request");
        return false;
    }

    const auto &request = packet.register_request();
    spdlog::info("Processing register request for user: {}", request.username());

    // 调用UserManager进行注册
    auto result = user_manager_.register_user(request.username(), request.password());

    // 创建响应
    auto response_packet = ProtocolHandler::create_packet(packet.version(), packet.sequence());
    // 返回 RegisterResponse*（指针），可以直接设置它的字段（如 success、message、user_id）。
    // 如果 Packet 内部还没有 RegisterResponse，这个函数会 创建一个新的 RegisterResponse 并返回指针。
    auto *response = response_packet.mutable_register_response();

    switch (result)
    {
    case UserManager::RegisterResult::SUCCESS:
    {
        response->set_success(true);
        response->set_message("User registered successfully");

        // 获取新创建的用户ID
        auto user_opt = user_manager_.find_user_by_username(request.username());
        if (user_opt.has_value())
        {
            response->set_user_id(user_opt->user_id);
        }

        spdlog::info("User registration successful: {}", request.username());
        break;
    }

    case UserManager::RegisterResult::USERNAME_EXISTS:
        response->set_success(false);
        response->set_message("Username already exists");
        spdlog::info("Registration failed - username exists: {}", request.username());
        break;

    case UserManager::RegisterResult::INVALID_USERNAME:
        response->set_success(false);
        response->set_message("Invalid username format");
        spdlog::info("Registration failed - invalid username: {}", request.username());
        break;

    case UserManager::RegisterResult::INVALID_PASSWORD:
        response->set_success(false);
        response->set_message("Invalid password format");
        spdlog::info("Registration failed - invalid password for user: {}", request.username());
        break;

    case UserManager::RegisterResult::DATABASE_ERROR:
        response->set_success(false);
        response->set_message("Internal server error");
        spdlog::error("Registration failed - database error for user: {}", request.username());
        break;
    }

    // 发送响应
    session->send_packet(response_packet);
    return true;
}