#pragma once

#include "message_handler.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <messages.pb.h>

// Forward declarations
class Session;

/**
 * @brief 消息路由器
 *
 * 负责将接收到的消息根据类型分发到相应的处理器。
 * 支持动态注册处理器，提供统一的错误处理和日志记录。
 */
class MessageRouter
{
public:
    /**
     * @brief 消息类型枚举
     * 用于标识不同的消息类型，便于路由分发
     * enum class是C++11 引入的强类型枚举和传统的 enum 相比，它有几个好处：
     * 1. 作用域明确：enum class 的枚举成员必须通过作用域解析符来访问，例如：MessageType::ECHO_REQUEST;避免了和其他枚举或全局常量冲突的问题。
     * 2. 普通的 enum 会被隐式转换成 int，可能引发 bug。enum class 不会自动转换，需要显式的 static_cast，保证了类型安全。
     * 3. 可指定底层类型：可以明确枚举底层类型，比如：enum class Color : uint8_t { Red, Green, Blue };
     *
     * MessageType 枚举类用来 区分消息的类型，方便 MessageRouter 选择正确的 MessageHandler 去处理。
     * MessageRouter::MessageType type = MessageRouter::MessageType::ECHO_REQUEST;

        if (type == MessageRouter::MessageType::ECHO_REQUEST) {
        // 调用对应的处理器
       }
     *
     */
    enum class MessageType
    {
        ECHO_REQUEST,
        USER_REGISTER,
        USER_LOGIN,
        // 未来可以添加更多消息类型
        // CHAT_MESSAGE,
        UNKNOWN
    };

    MessageRouter();
    ~MessageRouter() = default;

    /**
     * @brief 注册消息处理器
     * @param type 消息类型
     * @param handler 处理器实例
     */
    void register_handler(MessageType type, std::shared_ptr<MessageHandler> handler);

    /**
     * @brief 路由消息到对应处理器
     * @param packet 要处理的消息包
     * @param session 发送消息的会话
     * @return true 如果消息处理成功，false 如果失败
     */
    bool route_message(const Packet &packet, std::shared_ptr<Session> session);

    /**
     * @brief 获取注册的处理器数量
     * @return 处理器数量
     */
    size_t get_handler_count() const { return handlers_.size(); }

private:
    /**
     * @brief 从Packet确定消息类型
     * @param packet protobuf消息包
     * @return 消息类型
     */
    MessageType determine_message_type(const Packet &packet) const;

    /**
     * @brief 发送错误响应
     * @param error_code 错误代码
     * @param message 错误消息
     * @param sequence 序列号
     * @param session 会话
     */
    void send_error_response(uint32_t error_code, const std::string &message,
                             uint32_t sequence, std::shared_ptr<Session> session);

    /**
     * @brief 消息类型到字符串的转换（用于日志）
     * @param type 消息类型
     * @return 类型字符串
     */
    std::string message_type_to_string(MessageType type) const;

    /**
     * @brief 注册消息处理器
     *
     * 为什么使用 shared_ptr 而不是直接存对象？
     *
     * @note 主要原因有三点：
     *
     * 1. **共享所有权**
     *    - 一个 MessageHandler 可能会被多个地方使用，例如：
     *      - MessageRouter 保存一份
     *      - 日志系统 / 监控模块也保存一份
     *    - 使用 shared_ptr 可以保证对象只有在最后一个引用释放时才销毁，
     *      避免重复释放或悬空指针。
     *
     *    @code
     *    auto handler = std::make_shared<EchoHandler>();
     *    router.register_handler(MessageType::ECHO_REQUEST, handler);
     *    monitor.track_handler(handler);
     *    @endcode
     *
     * 2. **保持多态，避免对象切片**
     *    - MessageHandler 是一个基类，实际存放的是子类对象（EchoHandler、LoginHandler 等）。
     *    - 如果直接存对象，会发生对象切片（只保留基类部分），多态失效。
     *    - 使用 shared_ptr<MessageHandler> 可以保持子类的多态行为。
     *
     *    @code
     *    handlers_[MessageType::ECHO_REQUEST] = std::make_shared<EchoHandler>();
     *    handlers_[MessageType::USER_LOGIN]   = std::make_shared<LoginHandler>();
     *    @endcode
     *
     * 3. **支持动态替换**
     *    - register_handler() 可以随时更新某个消息类型对应的处理器。
     *    - 使用 shared_ptr 时，旧对象若没人引用会被安全销毁。
     *
     *    @code
     *    router.register_handler(MessageType::ECHO_REQUEST, std::make_shared<EchoHandler>());
     *    router.register_handler(MessageType::ECHO_REQUEST, std::make_shared<NewEchoHandler>());
     *    // 旧 EchoHandler 如果没人引用，会自动释放
     *    @endcode
     *
     * @param type 消息类型
     * @param handler 处理器实例（非空）
     */
    std::unordered_map<MessageType, std::shared_ptr<MessageHandler>> handlers_;
};