#include "database_manager.h"
#include <stdexcept>

DatabaseManager &DatabaseManager::get_instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initialize(const std::string &host,
                                 const std::string &user,
                                 const std::string &password,
                                 const std::string &database)
{
    std::lock_guard<std::mutex> lock(connection_mutex_);

    try
    {
        spdlog::info("Initializing database connection to {}", host);

        // 获取MySQL驱动实例
        driver_ = get_driver_instance();

        connection_url_ = host;
        username_ = user;
        password_ = password;
        database_name_ = database;

        // 测试连接 - 直接创建而不调用get_connection()以避免循环依赖
        std::unique_ptr<sql::Connection> test_conn(
            driver_->connect(connection_url_, username_, password_));
        test_conn->setSchema(database_name_);
        spdlog::debug("Test database connection successful");

        initialized_ = true;
        spdlog::info("Database connection initialized successfully");
        return true;
    }
    catch (sql::SQLException &e)
    {
        spdlog::error("Database initialization failed - SQL error: {} (code: {}, state: {})",
                      e.what(), e.getErrorCode(), e.getSQLState());
        return false;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Database initialization failed: {}", e.what());
        return false;
    }
}

std::unique_ptr<sql::Connection> DatabaseManager::get_connection()
{
    if (!initialized_)
    {
        spdlog::error("DatabaseManager not initialized");
        return nullptr;
    }

    try
    {
        // 创建新的连接
        std::unique_ptr<sql::Connection> conn(
            driver_->connect(connection_url_, username_, password_));

        // 选择数据库
        conn->setSchema(database_name_);

        spdlog::debug("Created new database connection");
        return conn;
    }
    catch (sql::SQLException &e)
    {
        spdlog::error("Failed to create database connection - SQL error: {} (code: {}, state: {})",
                      e.what(), e.getErrorCode(), e.getSQLState());
        return nullptr;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Failed to create database connection: {}", e.what());
        return nullptr;
    }
}

bool DatabaseManager::is_connected() const
{
    return initialized_;
}

void DatabaseManager::shutdown()
{
    std::lock_guard<std::mutex> lock(connection_mutex_);

    initialized_ = false;
    driver_ = nullptr;

    spdlog::info("Database manager shutdown complete");
}

DatabaseManager::~DatabaseManager()
{
    shutdown();
}