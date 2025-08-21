#pragma once

#include <string>
#include <memory>
#include <optional>
#include <cstdint>
#include "../database/database_manager.h"

/**
 * 用户信息结构体
 */
struct User
{
    int64_t user_id;
    std::string username;
    std::string password_hash;
    std::string created_at;
};

/**
 * UserManager类负责用户相关的业务逻辑
 * 包括用户注册、登录验证等功能
 */
class UserManager
{
public:
    static UserManager &get_instance();

    // 初始化用户管理器
    bool initialize();

    // 用户注册
    enum class RegisterResult
    {
        SUCCESS,
        USERNAME_EXISTS,
        INVALID_USERNAME,
        INVALID_PASSWORD,
        DATABASE_ERROR
    };
    RegisterResult register_user(const std::string &username, const std::string &password);

    // 用户登录验证
    enum class LoginResult
    {
        SUCCESS,
        USER_NOT_FOUND,
        WRONG_PASSWORD,
        DATABASE_ERROR
    };
    LoginResult authenticate_user(const std::string &username, const std::string &password, User &user_out);

    // 根据用户名查找用户
    std::optional<User> find_user_by_username(const std::string &username);

    // 根据用户ID查找用户
    std::optional<User> find_user_by_id(int64_t user_id);

private:
    UserManager() = default;
    ~UserManager() = default;

    // 禁用复制构造和赋值
    UserManager(const UserManager &) = delete;
    UserManager &operator=(const UserManager &) = delete;

    // 密码哈希相关
    std::string hash_password(const std::string &password);
    bool verify_password(const std::string &password, const std::string &hash);

    // 输入验证
    bool is_valid_username(const std::string &username);
    bool is_valid_password(const std::string &password);

    bool initialized_ = false;
};