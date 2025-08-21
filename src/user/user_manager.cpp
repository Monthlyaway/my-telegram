#include "user_manager.h"
#include <spdlog/spdlog.h>
#include <crypt.h>
#include <random>
#include <regex>
#include <ctime>

UserManager &UserManager::get_instance()
{
    static UserManager instance;
    return instance;
}

bool UserManager::initialize()
{
    if (initialized_)
    {
        return true;
    }

    spdlog::info("Initializing UserManager");

    // 检查数据库管理器是否已初始化
    if (!DatabaseManager::get_instance().is_connected())
    {
        spdlog::error("DatabaseManager not initialized");
        return false;
    }

    initialized_ = true;
    spdlog::info("UserManager initialized successfully");
    return true;
}

UserManager::RegisterResult UserManager::register_user(const std::string &username, const std::string &password)
{
    if (!initialized_)
    {
        spdlog::error("UserManager not initialized");
        return RegisterResult::DATABASE_ERROR;
    }

    // 验证输入
    if (!is_valid_username(username))
    {
        spdlog::warn("Invalid username: {}", username);
        return RegisterResult::INVALID_USERNAME;
    }

    if (!is_valid_password(password))
    {
        spdlog::warn("Invalid password for user: {}", username);
        return RegisterResult::INVALID_PASSWORD;
    }

    try
    {
        auto conn = DatabaseManager::get_instance().get_connection();
        if (!conn)
        {
            return RegisterResult::DATABASE_ERROR;
        }

        // 检查用户名是否已存在
        std::unique_ptr<sql::PreparedStatement> check_stmt(
            conn->prepareStatement("SELECT user_id FROM users WHERE username = ?"));
        check_stmt->setString(1, username);

        std::unique_ptr<sql::ResultSet> res(check_stmt->executeQuery());
        if (res->next())
        {
            spdlog::info("Username already exists: {}", username);
            return RegisterResult::USERNAME_EXISTS;
        }

        // 创建新用户
        std::string password_hash = hash_password(password);
        std::unique_ptr<sql::PreparedStatement> insert_stmt(
            conn->prepareStatement("INSERT INTO users (username, password_hash) VALUES (?, ?)"));
        insert_stmt->setString(1, username);
        insert_stmt->setString(2, password_hash);

        int affected_rows = insert_stmt->executeUpdate();
        if (affected_rows > 0)
        {
            spdlog::info("User registered successfully: {}", username);
            return RegisterResult::SUCCESS;
        }
        else
        {
            spdlog::error("Failed to insert user: {}", username);
            return RegisterResult::DATABASE_ERROR;
        }
    }
    catch (sql::SQLException &e)
    {
        spdlog::error("Database error during user registration: {} (code: {}, state: {})",
                      e.what(), e.getErrorCode(), e.getSQLState());
        return RegisterResult::DATABASE_ERROR;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error during user registration: {}", e.what());
        return RegisterResult::DATABASE_ERROR;
    }
}

UserManager::LoginResult UserManager::authenticate_user(const std::string &username,
                                                        const std::string &password,
                                                        User &user_out)
{
    if (!initialized_)
    {
        spdlog::error("UserManager not initialized");
        return LoginResult::DATABASE_ERROR;
    }

    try
    {
        auto user_opt = find_user_by_username(username);
        if (!user_opt.has_value())
        {
            spdlog::info("User not found: {}", username);
            return LoginResult::USER_NOT_FOUND;
        }

        User &user = user_opt.value();

        // 验证密码
        if (!verify_password(password, user.password_hash))
        {
            spdlog::info("Wrong password for user: {}", username);
            return LoginResult::WRONG_PASSWORD;
        }

        // 返回用户信息
        user_out = user;
        spdlog::info("User authenticated successfully: {}", username);
        return LoginResult::SUCCESS;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error during user authentication: {}", e.what());
        return LoginResult::DATABASE_ERROR;
    }
}

std::optional<User> UserManager::find_user_by_username(const std::string &username)
{
    try
    {
        auto conn = DatabaseManager::get_instance().get_connection();
        if (!conn)
        {
            return std::nullopt;
        }

        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT user_id, username, password_hash, created_at FROM users WHERE username = ?"));
        stmt->setString(1, username);

        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
        if (res->next())
        {
            User user;
            user.user_id = res->getInt64("user_id");
            user.username = res->getString("username");
            user.password_hash = res->getString("password_hash");
            user.created_at = res->getString("created_at");
            return user;
        }

        return std::nullopt;
    }
    catch (sql::SQLException &e)
    {
        spdlog::error("Database error finding user by username: {} (code: {}, state: {})",
                      e.what(), e.getErrorCode(), e.getSQLState());
        return std::nullopt;
    }
}

std::optional<User> UserManager::find_user_by_id(int64_t user_id)
{
    try
    {
        auto conn = DatabaseManager::get_instance().get_connection();
        if (!conn)
        {
            return std::nullopt;
        }

        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT user_id, username, password_hash, created_at FROM users WHERE user_id = ?"));
        stmt->setInt64(1, user_id);

        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
        if (res->next())
        {
            User user;
            user.user_id = res->getInt64("user_id");
            user.username = res->getString("username");
            user.password_hash = res->getString("password_hash");
            user.created_at = res->getString("created_at");
            return user;
        }

        return std::nullopt;
    }
    catch (sql::SQLException &e)
    {
        spdlog::error("Database error finding user by ID: {} (code: {}, state: {})",
                      e.what(), e.getErrorCode(), e.getSQLState());
        return std::nullopt;
    }
}

std::string UserManager::hash_password(const std::string &password)
{
    // 生成随机salt
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 61); // 0-9, A-Z, a-z

    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string salt = "$6$rounds=5000$";
    for (int i = 0; i < 16; ++i)
    {
        salt += chars[dis(gen)];
    }
    salt += "$";

    // 使用crypt进行哈希
    char *hashed = crypt(password.c_str(), salt.c_str());
    if (hashed == nullptr)
    {
        throw std::runtime_error("Password hashing failed");
    }

    return std::string(hashed);
}

bool UserManager::verify_password(const std::string &password, const std::string &hash)
{
    char *verified = crypt(password.c_str(), hash.c_str());
    if (verified == nullptr)
    {
        return false;
    }

    return hash == std::string(verified);
}

bool UserManager::is_valid_username(const std::string &username)
{
    // 用户名长度 3-50 字符，只允许字母、数字和下划线
    if (username.length() < 3 || username.length() > 50)
    {
        return false;
    }

    std::regex username_regex("^[a-zA-Z0-9_]+$");
    return std::regex_match(username, username_regex);
}

bool UserManager::is_valid_password(const std::string &password)
{
    // 密码长度 6-50 字符
    return password.length() >= 6 && password.length() <= 50;
}