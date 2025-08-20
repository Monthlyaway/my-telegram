# MyTelegram - 即时通讯系统

基于C++17的高性能即时通讯系统，采用现代异步I/O架构和企业级技术栈。

## 当前状态：阶段3 - Protobuf协议集成 ✅

### 已实现功能
- **配置管理**：JSON配置文件支持，灵活的服务器参数配置
- **异步网络**：基于Asio的高性能TCP服务器
- **会话管理**：多客户端并发连接处理
- **Protobuf协议**：结构化消息通信，4字节长度+Protobuf数据帧格式
- **协议处理**：完整的序列化/反序列化、版本检查和错误处理
- **回显服务**：基于Protobuf的EchoRequest/EchoResponse机制
- **日志系统**：spdlog双输出（控制台+文件轮转）
- **优雅关闭**：信号处理和资源清理
- **Python客户端**：完整的测试客户端支持多种测试场景

### 技术栈
- **核心语言**：C++17
- **网络库**：Asio (Standalone)
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

### 3. 测试Protobuf通信
```bash
# 配置Python环境
cd /home/will/my-telegram
python3 -m venv venv
source venv/bin/activate
pip install protobuf

# 生成Python protobuf文件
protoc --python_out=tests protos/messages.proto

# 运行完整的协议测试
python tests/test_client.py
```

### 4. 传统测试方式（仍然支持）
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
│   ├── protocol/         # 协议处理模块
│   │   ├── protocol_handler.h
│   │   └── protocol_handler.cpp
│   └── server/           # 服务器核心模块
│       ├── server.h      # 服务器主类
│       ├── server.cpp
│       ├── session.h     # 会话管理
│       └── session.cpp
├── tests/
│   ├── test_client.py    # Python协议测试客户端
│   ├── dependency_test.cpp # 依赖测试程序
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
- ✅ 服务器成功启动在8080端口
- ✅ Protobuf协议通信正常工作
- ✅ 帧格式解析正确（4字节长度+数据）
- ✅ 协议版本检查和错误处理完善
- ✅ Python测试客户端全部测试通过（5/5）
- ✅ 支持Unicode和长消息处理
- ✅ 多客户端并发连接支持
- ✅ 日志记录完整（控制台+文件）
- ✅ 优雅关闭机制工作正常

## 开发计划

### ✅ 已完成阶段
- **阶段1**：环境搭建与依赖验证
- **阶段2**：基础TCP回声服务器  
- **阶段3**：Protobuf协议集成

### 下一阶段：阶段4 - 消息路由系统
- 建立可扩展的消息处理架构
- 实现MessageRouter消息分发机制
- 创建处理器接口和EchoHandler示例
- 添加SessionManager会话管理器

### 后续阶段
- 阶段5：用户系统（MySQL + 注册登录）
- 阶段6：私聊功能
- 阶段7：群聊功能

## 架构特点
- **异步I/O**：基于事件驱动的高并发处理
- **Protobuf协议**：类型安全的结构化消息通信
- **帧协议**：4字节长度头+数据体，防止粘包问题
- **版本控制**：协议版本检查，支持向后兼容演进
- **模块化设计**：清晰的分层架构，便于扩展
- **企业级日志**：结构化日志输出，便于监控调试
- **配置驱动**：灵活的JSON配置，支持运行时调整
- **资源管理**：RAII和智能指针，内存安全
- **完整测试**：Python客户端提供全面的协议测试

---
*构建于 2025-08-20 | Stage 3 完成*