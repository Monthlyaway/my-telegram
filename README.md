# MyTelegram - 即时通讯系统

基于C++17的高性能即时通讯系统，采用现代异步I/O架构和企业级技术栈。

## 当前状态：阶段5 - 用户系统 ✅

### 已实现功能
- **配置管理**：JSON配置文件支持，灵活的服务器参数配置
- **异步网络**：基于Asio的高性能TCP服务器
- **消息路由系统**：MessageRouter实现可扩展的消息分发架构
- **处理器接口**：MessageHandler统一接口，支持多种消息类型处理
- **会话管理器**：SessionManager线程安全的会话生命周期管理
- **用户系统**：完整的用户注册、登录和身份认证功能
- **数据库集成**：MySQL数据库存储，线程安全的连接管理
- **密码安全**：bcrypt哈希算法，安全的密码存储和验证
- **用户管理**：RegisterHandler和LoginHandler处理用户相关请求
- **会话认证**：Session级别的用户状态管理和认证标记
- **Protobuf协议**：结构化消息通信，4字节长度+Protobuf数据帧格式
- **协议处理**：完整的序列化/反序列化、版本检查和错误处理
- **回显服务**：通过EchoHandler实现的路由化消息处理
- **日志系统**：spdlog双输出（控制台+文件轮转），详细调试信息
- **优雅关闭**：信号处理、资源清理和会话管理
- **多客户端支持**：并发连接处理，会话统计和监控
- **Python测试套件**：完整的协议、路由器和用户系统测试客户端

### 技术栈
- **核心语言**：C++17
- **网络库**：Asio (Standalone)
- **数据库**：MySQL 8.0+ (mysql-connector-cpp)
- **密码哈希**：bcrypt (crypt库)
- **消息协议**：Google Protobuf
- **日志库**：spdlog
- **配置格式**：nlohmann/json
- **构建系统**：CMake
- **测试环境**：Python 3.8+
- **开发环境**：WSL Ubuntu

## 快速开始

### 1. 构建项目
```bash
cd /home/will/my-telegram
rm -rf build  # 清理旧版本
mkdir build && cd build
cmake .. && make -j12
```

### 2. 运行服务器
```bash
# 在项目根目录下运行，使用默认配置
cd /home/will/my-telegram/build
./im_server ../config.json
```

### 3. 初始化数据库
```bash
# 连接到MySQL（使用你的凭据）
mysql -u will -p

# 在MySQL中执行以下命令
USE testdb;
SOURCE /home/will/my-telegram/database/init_user_system.sql;
```

### 4. 测试系统功能
```bash
# 配置Python环境
cd /home/will/my-telegram
python3 -m venv venv
source venv/bin/activate
pip install protobuf

# 生成Python protobuf文件
protoc --cpp_out=. --python_out=tests protos/messages.proto

# 运行用户系统测试（阶段5新增）
python tests/test_user_system.py

# 运行路由器系统测试
python tests/test_router.py

# 运行原有的协议测试（兼容性测试）
python tests/test_client.py
```

### 5. 传统测试方式（仍然支持）
```bash
# telnet测试（仅适用于简单文本，不支持Protobuf协议）
telnet localhost 8080
```

## 项目结构
```
my-telegram/
├── config.json           # 服务器配置文件
├── CMakeLists.txt        # 构建配置
├── protos/
│   └── messages.proto    # Protobuf协议定义
├── src/
│   ├── main.cpp          # 程序入口
│   ├── config/           # 配置管理模块
│   │   ├── config.h
│   │   └── config.cpp
│   ├── database/         # 数据库管理模块
│   │   ├── database_manager.h
│   │   └── database_manager.cpp
│   ├── user/             # 用户管理模块
│   │   ├── user_manager.h
│   │   └── user_manager.cpp
│   ├── protocol/         # 协议处理模块
│   │   ├── protocol_handler.h
│   │   └── protocol_handler.cpp
│   ├── router/           # 消息路由模块
│   │   ├── message_handler.h      # 处理器接口
│   │   ├── message_handler.cpp
│   │   ├── message_router.h       # 消息路由器
│   │   ├── message_router.cpp
│   │   ├── register_handler.h     # 用户注册处理器
│   │   ├── register_handler.cpp
│   │   ├── login_handler.h        # 用户登录处理器
│   │   └── login_handler.cpp
│   └── server/           # 服务器核心模块
│       ├── server.h      # 服务器主类
│       ├── server.cpp
│       ├── session.h     # 会话处理
│       ├── session.cpp
│       ├── session_manager.h      # 会话管理器
│       └── session_manager.cpp
├── database/
│   └── init_user_system.sql # 数据库初始化脚本
├── tests/
│   ├── test_user_system.py  # 用户系统测试客户端
│   ├── test_router.py       # 路由器测试客户端
│   ├── test_client.py       # 协议测试客户端（兼容性）
│   ├── dependency_test.cpp  # 依赖测试程序
│   └── CMakeLists.txt
├── logs/                 # 日志输出目录
└── build/                # 构建输出目录
```

## 配置说明
`config.json` 支持以下配置：
```json
{
  "server": {
    "host": "0.0.0.0",        // 监听地址
    "port": 8080,             // 监听端口
    "max_connections": 1000,   // 最大连接数
    "worker_threads": 4        // 工作线程数
  },
  "logging": {
    "level": "info",          // 日志级别
    "file": "logs/im_server.log", // 日志文件
    "max_size_mb": 100,       // 文件大小限制
    "max_files": 5            // 保留文件数量
  }
}
```

## 验收结果 ✅

### 阶段5：用户系统
- ✅ 数据库连接成功初始化（MySQL 8.0）
- ✅ 用户注册功能完整实现（RegisterHandler）
- ✅ 用户登录功能完整实现（LoginHandler）
- ✅ 密码安全性保障（bcrypt哈希算法）
- ✅ 用户名验证（3-50字符，字母数字下划线）
- ✅ 密码验证（6-50字符）
- ✅ 重复用户名检测和拒绝
- ✅ 错误密码检测和拒绝
- ✅ 不存在用户检测和拒绝
- ✅ Session级别的用户认证状态管理
- ✅ 用户系统测试客户端全部通过（6/6）

### 基础功能
- ✅ 服务器成功启动在8080端口
- ✅ MessageRouter系统正确初始化（3个处理器）
- ✅ 消息正确路由到对应处理器（Echo/Register/Login）
- ✅ SessionManager管理多客户端会话
- ✅ 未知消息类型返回正确错误响应
- ✅ 协议版本检查和错误处理完善
- ✅ 路由器测试客户端保持兼容性（6/6）
- ✅ 原有协议测试保持兼容性（5/5）
- ✅ 支持Unicode和长消息处理
- ✅ 线程安全的会话统计和管理
- ✅ 详细的调试日志和监控信息
- ✅ 优雅关闭机制和资源清理（Ctrl+C快速退出）

## 开发计划

### ✅ 已完成阶段
- **阶段1**：环境搭建与依赖验证
- **阶段2**：基础TCP回声服务器  
- **阶段3**：Protobuf协议集成
- **阶段4**：消息路由系统
- **阶段5**：用户系统

### 下一阶段：阶段6 - 私聊功能
- 实现用户间的私聊消息发送
- 添加在线状态管理
- 扩展协议支持私聊相关消息
- 实现消息存储和离线消息推送

### 后续阶段
- 阶段7：群聊功能
- 阶段8：文件传输
- 阶段9：推送通知

## 架构特点
- **异步I/O**：基于事件驱动的高并发处理
- **消息路由架构**：可扩展的MessageRouter分发系统
- **处理器模式**：统一的MessageHandler接口，支持多消息类型
- **会话管理**：线程安全的SessionManager，支持并发连接和用户认证
- **用户系统**：完整的注册登录、密码安全和数据持久化
- **数据库集成**：MySQL连接池、事务管理和SQL注入防护
- **Protobuf协议**：类型安全的结构化消息通信
- **帧协议**：4字节长度头+数据体，防止粘包问题
- **版本控制**：协议版本检查，支持向后兼容演进
- **模块化设计**：清晰的分层架构，便于扩展
- **企业级日志**：结构化日志输出，详细调试信息
- **配置驱动**：灵活的JSON配置，支持运行时调整
- **资源管理**：RAII和智能指针，内存安全
- **完整测试**：多层次Python测试客户端，全面验证功能

---
*构建于 2025-08-21 | Stage 5 完成*