#pragma once

#include <mysql/jdbc.h>
#include <memory>
#include <string>
#include <mutex>
#include <spdlog/spdlog.h>

/**
 * DatabaseManager类负责管理MySQL连接和数据库操作
 * 提供线程安全的数据库访问接口
 */
class DatabaseManager
{
public:
    static DatabaseManager &get_instance();

    // 初始化数据库连接
    bool initialize(const std::string &host = "tcp://127.0.0.1:3306",
                    const std::string &user = "will",
                    const std::string &password = "abcd1234",
                    const std::string &database = "testdb");

    // 获取数据库连接
    std::unique_ptr<sql::Connection> get_connection();

    // 检查数据库连接状态
    bool is_connected() const;

    // 关闭数据库连接
    void shutdown();

private:
    DatabaseManager() = default;
    ~DatabaseManager();

    // 禁用复制构造和赋值
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    sql::Driver *driver_ = nullptr;
    std::string connection_url_;
    std::string username_;
    std::string password_;
    std::string database_name_;
    bool initialized_ = false;
    mutable std::mutex connection_mutex_;
};