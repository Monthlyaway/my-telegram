#pragma once

#include "message_handler.h"
#include "../user/user_manager.h"

/**
 * RegisterHandler 处理用户注册请求
 */
class RegisterHandler : public MessageHandler
{
public:
    RegisterHandler();
    ~RegisterHandler() override = default;

    bool handle(const Packet &packet, std::shared_ptr<Session> session) override;
    std::string get_handler_name() const override { return "RegisterHandler"; }

private:
    UserManager &user_manager_;
};