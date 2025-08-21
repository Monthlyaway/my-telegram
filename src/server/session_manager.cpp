#include "session_manager.h"
#include <spdlog/spdlog.h>
#include <sstream>

SessionManager &SessionManager::get_instance()
{
    static SessionManager instance;
    return instance;
}

bool SessionManager::register_session(std::shared_ptr<Session> session)
{
    if (!session)
    {
        spdlog::error("Cannot register null session");
        return false;
    }

    std::lock_guard<std::mutex> lock(sessions_mutex_);

    // 检查是否已经存在
    if (active_sessions_.find(session) != active_sessions_.end())
    {
        spdlog::warn("Session already registered");
        return false;
    }

    // 注册新会话
    active_sessions_.insert(session);
    size_t current_count = active_sessions_.size();

    spdlog::info("Session registered, active sessions: {}", current_count);

    // 更新最大会话数统计
    update_max_session_count(current_count);

    return true;
}

bool SessionManager::unregister_session(std::shared_ptr<Session> session)
{
    if (!session)
    {
        spdlog::error("Cannot unregister null session");
        return false;
    }

    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto it = active_sessions_.find(session);
    if (it == active_sessions_.end())
    {
        spdlog::debug("Session not found for unregistration");
        return false;
    }

    // 注销会话
    active_sessions_.erase(it);
    size_t current_count = active_sessions_.size();

    spdlog::info("Session unregistered, active sessions: {}", current_count);

    return true;
}

bool SessionManager::is_session_registered(std::shared_ptr<Session> session) const
{
    if (!session)
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(sessions_mutex_);
    return active_sessions_.find(session) != active_sessions_.end();
}

size_t SessionManager::get_active_session_count() const
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    return active_sessions_.size();
}

void SessionManager::shutdown_all_sessions()
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    if (active_sessions_.empty())
    {
        spdlog::info("No active sessions to shutdown");
        return;
    }

    spdlog::info("Shutting down {} active sessions", active_sessions_.size());

    // 遍历所有会话并优雅关闭
    for (auto &session : active_sessions_)
    {
        try
        {
            // 关闭socket连接
            if (session->socket_.is_open())
            {
                session->socket_.close();
            }
        }
        catch (const std::exception &e)
        {
            spdlog::warn("Error closing session: {}", e.what());
        }
    }

    // 清空会话列表
    active_sessions_.clear();
    spdlog::info("All sessions shutdown completed");
}

std::string SessionManager::get_session_stats() const
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    std::ostringstream oss;
    oss << "SessionManager Stats: "
        << "Active=" << active_sessions_.size()
        << ", MaxEver=" << max_session_count_.load();

    return oss.str();
}

void SessionManager::update_max_session_count(size_t current_count)
{
    size_t current_max = max_session_count_.load();
    while (current_count > current_max)
    {
        if (max_session_count_.compare_exchange_weak(current_max, current_count))
        {
            break;
        }
    }
}