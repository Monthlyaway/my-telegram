#pragma once

#include "message_handler.h"
#include "../user/user_manager.h"

/**
 * LoginHandler 处理用户登录请求
 */
class LoginHandler : public MessageHandler
{
public:
    LoginHandler();
    ~LoginHandler() override = default;

    bool handle(const Packet &packet, std::shared_ptr<Session> session) override;
    std::string get_handler_name() const override { return "LoginHandler"; }

private:
    UserManager &user_manager_;
};