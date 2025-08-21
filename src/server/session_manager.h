#pragma once

#include <memory>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include "session.h"

/**
 * @brief 会话管理器
 *
 * 负责管理所有活跃的Session连接，提供线程安全的会话注册、注销和查找功能。
 * 使用单例模式确保全局只有一个SessionManager实例。
 *
 * 没有SessionManager时，Server无法知道当前有多少活跃连接，历史最大并发连接数是多少，哪些客户端连接了多少时间
 * Server关闭的时候，无法主动关闭所有活跃连接，客户端长时间等待，无法限制最大的并发连接数，拒绝过多连接以保护服务器，无法向所有的客户端广播信息。
 * 
 * 为什么是单例：
  3. 跨模块访问便利性

  // Session析构时需要注销
  Session::~Session() {
      // 方便：无需传递SessionManager引用
      SessionManager::get_instance().unregister_session(shared_from_this());
  }

  // 未来消息路由器可能需要广播
  void MessageRouter::broadcast_system_message(const std::string& msg) {
      // 方便：直接访问全局会话管理器
      SessionManager::get_instance().broadcast_to_all(msg);
  }
 */
class SessionManager
{
public:
    /**
     * @brief 获取SessionManager单例实例
     * @return SessionManager实例引用
     */
    static SessionManager &get_instance();

    // 禁用拷贝构造和赋值
    SessionManager(const SessionManager &) = delete;
    SessionManager &operator=(const SessionManager &) = delete;

    /**
     * @brief 注册新的会话
     * @param session 要注册的会话
     * @return true 如果注册成功，false 如果会话已存在或为null
     */
    bool register_session(std::shared_ptr<Session> session);

    /**
     * @brief 注销会话
     * @param session 要注销的会话
     * @return true 如果注销成功，false 如果会话不存在
     */
    bool unregister_session(std::shared_ptr<Session> session);

    /**
     * @brief 检查会话是否已注册
     * @param session 要检查的会话
     * @return true 如果会话已注册，false 否则
     */
    bool is_session_registered(std::shared_ptr<Session> session) const;

    /**
     * @brief 获取当前活跃会话数量
     * @return 活跃会话数量
     */
    size_t get_active_session_count() const;

    /**
     * @brief 获取最大会话数统计
     * @return 历史最大会话数
     */
    size_t get_max_session_count() const { return max_session_count_.load(); }

    /**
     * @brief 关闭所有会话
     * 在服务器关闭时调用，优雅地关闭所有活跃连接
     */
    void shutdown_all_sessions();

    /**
     * @brief 获取会话统计信息字符串
     * @return 包含会话统计的字符串
     */
    std::string get_session_stats() const;

private:
    SessionManager() = default;
    ~SessionManager() = default;

    /**
     * @brief 更新最大会话数统计
     * @param current_count 当前会话数
     */
    void update_max_session_count(size_t current_count);

    // 线程安全的会话容器
    /**
     * mutable 的作用是：即使 SessionManager 的成员函数是 const（比如 is_session_registered），也允许修改这个 mutex，这样才能在 const 方法里加锁。
     * 用于保证多个线程同时操作 active_sessions_ 时不会出错。
     */
    mutable std::mutex sessions_mutex_;
    std::unordered_set<std::shared_ptr<Session>> active_sessions_;

    // 统计信息
    /**
     * @brief 这是一个 原子变量，用来统计 历史上同时在线的最大会话数。
     * 因为这个数可能在不同线程里同时更新（比如一个线程新注册会话，另一个线程也在注册），如果不用原子操作，就会有并发写入的竞态问题。
     * 在 register_session 里，每次插入新会话时，会调用 update_max_session_count(current_count)
     * 
     */
    std::atomic<size_t> max_session_count_{0};
};