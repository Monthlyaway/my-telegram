# MyTelegram - 即时通讯系统

基于C++17的高性能即时通讯系统，采用现代异步I/O架构和企业级技术栈。

## 当前状态：阶段2 - 基础TCP回声服务器 ✅

### 已实现功能
- **配置管理**：JSON配置文件支持，灵活的服务器参数配置
- **异步网络**：基于Asio的高性能TCP服务器
- **会话管理**：多客户端并发连接处理
- **回声功能**：完整的消息接收和回显机制
- **日志系统**：spdlog双输出（控制台+文件轮转）
- **优雅关闭**：信号处理和资源清理

### 技术栈
- **核心语言**：C++17
- **网络库**：Asio (Standalone)
- **日志库**：spdlog
- **配置格式**：nlohmann/json
- **构建系统**：CMake
- **开发环境**：WSL Ubuntu

## 快速开始

### 1. 构建项目
```bash
cd /home/will/my-telegram
mkdir -p build && cd build
cmake .. && make -j12
```

### 2. 运行服务器
```bash
# 在项目根目录下运行，使用默认配置
cd /home/will/my-telegram/build
./im_server ../config.json
```

### 3. 测试连接
```bash
# 使用telnet测试
telnet localhost 8080

# 或使用netcat
nc localhost 8080
```

输入任何文字按回车，服务器会回显相同内容。

## 项目结构
```
my-telegram/
├── config.json           # 服务器配置文件
├── CMakeLists.txt        # 构建配置
├── src/
│   ├── main.cpp          # 程序入口
│   ├── config/           # 配置管理模块
│   │   ├── config.h
│   │   └── config.cpp
│   └── server/           # 服务器核心模块
│       ├── server.h      # 服务器主类
│       ├── server.cpp
│       ├── session.h     # 会话管理
│       └── session.cpp
├── tests/                # 测试代码
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
- ✅ telnet连接测试通过
- ✅ 多客户端并发连接支持
- ✅ 消息回显功能正常
- ✅ 日志记录完整（控制台+文件）
- ✅ 优雅关闭机制工作正常

## 开发计划

### 下一阶段：阶段3 - Protobuf协议集成
- 引入Google Protobuf消息协议
- 实现4字节长度+Protobuf数据的帧格式
- 添加协议版本检查和错误处理
- 创建Python测试客户端

### 后续阶段
- 阶段4：消息路由系统
- 阶段5：用户系统（MySQL + 注册登录）
- 阶段6：私聊功能
- 阶段7：群聊功能

## 架构特点
- **异步I/O**：基于事件驱动的高并发处理
- **模块化设计**：清晰的分层架构，便于扩展
- **企业级日志**：结构化日志输出，便于监控调试
- **配置驱动**：灵活的JSON配置，支持运行时调整
- **资源管理**：RAII和智能指针，内存安全

---
*构建于 2025-08-20 | Stage 2 完成*