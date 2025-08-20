### 完整技术栈

#### **依赖管理**
* **包管理器**: `apt` (WSL Ubuntu) - 使用系统包管理器安装开发依赖，简化环境配置

#### **阶段 0-2 核心技术栈**

* **核心开发语言**:
    * `C++17` - 我们将利用其现代化的语言特性来构建项目。
* **构建系统**:
    * `CMake` (≥3.15) - 负责项目的配置、编译和链接。
* **网络库**:
    * `Asio (Standalone version)` - 作为项目异步I/O的核心，处理所有TCP网络操作。
* **数据库**:
    * `MySQL 8.0+` - 数据持久化
    * `mysql-connector-cpp` - 官方C++连接器
* **序列化库**:
    * `Google Protocol Buffers (Protobuf)` - 用于定义消息格式和网络协议。
* **日志库**:
    * `spdlog` - 用于在开发和调试过程中输出结构化的、高性能的日志。
* **密码处理**:
    * `bcrypt` - 安全的密码哈希库
* **配置管理**:
    * `nlohmann/json` - 用于解析配置文件
* **测试框架**:
    * `Google Test (gtest)` - 单元测试框架
    * `Python 3.8+` - 用于编写集成测试客户端

---

## **核心协议设计**

### **协议架构原则**

1. **版本兼容性**: 所有消息包含版本字段，支持向后兼容的协议演进
2. **统一错误处理**: 标准化的错误响应格式，包含错误码和描述
3. **类型安全**: 使用Protobuf的强类型系统和oneof确保消息类型安全
4. **可扩展性**: 预留字段编号和扩展点，便于后续功能添加

### **传输层协议**

**帧格式**: `[4字节长度][Protobuf数据]`
- **长度字段**: 32位大端字节序，表示后续Protobuf数据的字节长度
- **最大帧长度**: 1MB (1048576 bytes)，防止内存攻击
- **字节序**: 网络字节序(大端)

### **应用层协议结构**

```protobuf
message Packet {
    uint32 version = 1;      // 协议版本，当前为1
    uint32 sequence = 2;     // 可选的序列号，用于请求-响应匹配
    oneof payload {
        // 系统消息 (1-99)
        EchoRequest echo_request = 10;
        EchoResponse echo_response = 11;
        HeartbeatRequest heartbeat_request = 20;
        HeartbeatResponse heartbeat_response = 21;
        
        // 用户管理 (100-199)
        RegisterRequest register_request = 100;
        RegisterResponse register_response = 101;
        LoginRequest login_request = 102;
        LoginResponse login_response = 103;
        
        // 消息相关 (200-299)
        SendMessageRequest send_message_request = 200;
        SendMessageResponse send_message_response = 201;
        MessagePush message_push = 202;
        
        // 错误响应 (999)
        ErrorResponse error = 999;
    }
}
```

### **错误处理标准**

```protobuf
message ErrorResponse {
    uint32 error_code = 1;
    string message = 2;
    map<string, string> details = 3;  // 额外的错误详情
}
```

**标准错误码**:
- `1000-1099`: 协议错误 (版本不匹配、格式错误等)
- `2000-2099`: 认证错误 (登录失败、会话过期等)  
- `3000-3099`: 业务逻辑错误 (用户不存在、权限不足等)
- `4000-4099`: 系统错误 (数据库错误、内部异常等)

### **消息ID和幂等性设计**

- **客户端消息ID**: UUID格式，确保重复检测和幂等性
- **服务器消息ID**: 数据库自增ID，全局唯一
- **序列号**: 用于请求-响应匹配，客户端生成

---

## **测试和验收策略**

### **测试框架和工具**

1. **单元测试**: Google Test (gtest) - 测试单个类和函数
2. **集成测试**: Python测试客户端 - 测试完整的客户端-服务器交互
3. **性能测试**: 简单的压力测试脚本 - 验证并发处理能力
4. **手动测试**: telnet/netcat - 快速验证基本功能

### **简化测试策略**

每个阶段只做**一个核心验证**，重点是功能跑通。

#### **各阶段验证方式**
- **阶段1**: 编译成功，输出库版本信息
- **阶段2**: `telnet localhost 8080` 测试回声
- **阶段3**: Python客户端发送protobuf消息
- **阶段4**: 测试消息路由到不同处理器
- **阶段5**: 完整的注册-登录流程
- **阶段6**: 两用户私聊场景
- **阶段7**: 多用户群聊场景

#### **构建和测试脚本**
```bash
# build.sh - 简单构建
#!/bin/bash
mkdir -p build && cd build
cmake .. && make -j4

# 运行服务器
./im_server
```

#### **Python测试客户端模板**
```python
# test_client.py - 用于各阶段测试
import socket
import struct
import time
from messages_pb2 import *

def send_message(host, port, packet):
    """发送protobuf消息并接收响应"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    
    # 发送
    data = packet.SerializeToString()
    sock.send(struct.pack('!I', len(data)) + data)
    
    # 接收
    length = struct.unpack('!I', sock.recv(4))[0]
    response = sock.recv(length)
    sock.close()
    
    return response

# 各阶段测试函数
def test_stage3_echo():
    """阶段3: Protobuf回显测试"""
    packet = Packet(version=1, echo_request=EchoRequest(content="hello"))
    response = send_message('localhost', 8080, packet)
    print("Echo test:", response)

def test_stage5_user():
    """阶段5: 用户注册登录测试"""
    # 注册
    reg_packet = Packet(version=1, register_request=RegisterRequest(
        username="alice", password="123456"))
    response = send_message('localhost', 8080, reg_packet)
    print("Register:", response)
    
    # 登录
    login_packet = Packet(version=1, login_request=LoginRequest(
        username="alice", password="123456"))
    response = send_message('localhost', 8080, login_packet)
    print("Login:", response)
```

#### **基本性能要求**
- 支持10个并发连接
- 消息响应延迟 < 100ms  
- 不能有明显的内存泄漏

---

## **环境配置和部署**

## **WSL Ubuntu 开发环境配置**

### **一键环境安装脚本**
```bash
#!/bin/bash
# install_deps.sh - WSL Ubuntu 环境配置

# 更新系统
sudo apt update && sudo apt upgrade -y

# 基础开发工具
sudo apt install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    gdb \
    valgrind

# 项目依赖库
sudo apt install -y \
    libasio-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libspdlog-dev \
    nlohmann-json3-dev \
    libmysqlcppconn-dev \
    libssl-dev \
    libcrypt-dev

# 测试工具
sudo apt install -y \
    netcat-openbsd \
    telnet \
    python3 \
    python3-pip \
    mysql-server \
    mysql-client

# Python protobuf支持
pip3 install protobuf

echo "环境配置完成！"
```

### **数据库初始化**
```bash
# setup_db.sh - 数据库初始化脚本
#!/bin/bash

# 启动MySQL服务
sudo service mysql start

# 创建数据库和用户
mysql
CREATE DATABASE IF NOT EXISTS im_chat DEFAULT CHARACTER SET utf8mb4;
CREATE USER IF NOT EXISTS 'im_user'@'localhost' IDENTIFIED BY 'Im_password1234!';
GRANT ALL PRIVILEGES ON im_chat.* TO 'im_user'@'localhost';
FLUSH PRIVILEGES;
SHOW DATABASES;
EOF

echo "数据库设置完成！"
echo "连接信息："
echo "  数据库: im_chat"
echo "  用户: im_user"
echo "  密码: im_password"
```

### **项目结构**
```
my-telegram/
├── CMakeLists.txt          # 构建配置
├── config.json            # 配置文件
├── src/
│   ├── main.cpp           # 主程序
│   ├── server/            # 服务器模块
│   ├── protocol/          # 协议处理
│   ├── database/          # 数据库操作
│   └── utils/             # 工具类
├── protos/
│   └── messages.proto     # 协议定义
├── tests/
│   └── test_client.py     # 测试客户端
├── scripts/
│   ├── install_deps.sh    # 环境安装
│   ├── setup_db.sh        # 数据库初始化
│   └── build.sh           # 构建脚本
└── schema.sql             # 数据库表结构
```

#### **配置文件设计 (`config/config.json`)**
```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "max_connections": 1000,
    "worker_threads": 4
  },
  "database": {
    "host": "localhost",
    "port": 3306,
    "database": "im_chat",
    "username": "im_user",
    "password": "im_password",
    "pool_size": 10,
    "connection_timeout": 30
  },
  "logging": {
    "level": "info",
    "file": "logs/im_server.log",
    "max_size_mb": 100,
    "max_files": 5
  },
  "security": {
    "bcrypt_rounds": 12,
    "session_timeout": 3600
  }
}
```

### **构建系统配置**

#### **CMake配置模板 (`CMakeLists.txt`)**
```cmake
cmake_minimum_required(VERSION 3.15)
project(im_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用pkg-config查找依赖
find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

# 查找系统依赖
pkg_check_modules(SPDLOG REQUIRED spdlog)
pkg_check_modules(MYSQLCPPCONN REQUIRED mysqlcppconn)

# 生成protobuf文件
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS protos/messages.proto)

# 主可执行文件
add_executable(im_server
    src/main.cpp
    ${PROTO_SRCS}
    # 其他源文件...
)

# 包含目录
target_include_directories(im_server PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}  # for protobuf generated files
    ${SPDLOG_INCLUDE_DIRS}
    ${MYSQLCPPCONN_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(im_server
    ${Protobuf_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    ${MYSQLCPPCONN_LIBRARIES}
    Threads::Threads
    crypt  # for password hashing
)

# 编译选项
target_compile_options(im_server PRIVATE ${SPDLOG_CFLAGS_OTHER})

# 单元测试（简化）
find_package(GTest)
if(GTest_FOUND)
    add_executable(im_tests
        tests/unit/test_main.cpp
        # 测试源文件...
    )
    target_link_libraries(im_tests GTest::gtest GTest::gtest_main)
endif()
```





---

## **阶段 1: 环境搭建与依赖验证**

### **目标**
验证WSL Ubuntu开发环境，确保所有依赖库正确安装和链接。

### **实现内容**
1. **构建系统配置** (`CMakeLists.txt`)
   - 配置C++17标准
   - 查找和链接所有系统依赖库
   - 设置正确的编译选项

2. **最小化程序** (`main.cpp`)
   - 初始化spdlog日志系统
   - 输出库版本和启动信息
   - 验证所有依赖库可正常使用

### **验收标准**
- 执行 `mkdir build && cd build && cmake .. && make` 编译成功
- 运行 `./im_server` 输出所有库的版本信息
- 日志格式正确，无链接错误

---

## **阶段 2: 基础TCP回声服务器**

### **目标**
建立基本的网络连接和数据传输能力，实现字符串回显功能。

### **实现内容**
1. **配置系统** (`Config` class + `config.json`)
   - JSON配置文件：端口、日志级别
   - 简单的配置解析类

2. **TCP服务器** (`Server` class)
   - Asio异步TCP服务器
   - 监听指定端口，接受客户端连接

3. **会话管理** (`Session` class)
   - 每个连接一个Session对象
   - 异步读取文本行，原样回显
   - 自动处理连接断开

### **验收标准**
- 服务器启动成功，监听8080端口
- 使用 `telnet localhost 8080` 连接
- 输入文字+回车，服务器返回相同内容
- 支持多个客户端同时连接

---

## **阶段 3: Protobuf协议集成**

### **目标**
引入结构化的消息协议，实现Protobuf消息的序列化和反序列化。

### **实现内容**
1. **协议定义** (`messages.proto`)
   ```protobuf
   message Packet {
       uint32 version = 1;
       oneof payload {
           EchoRequest echo_request = 10;
           EchoResponse echo_response = 11;
           ErrorResponse error = 99;
       }
   }
   ```

2. **协议处理** (`ProtocolHandler` class)
   - 4字节长度 + Protobuf数据的帧格式
   - 序列化/反序列化方法
   - 协议版本检查

3. **升级Session类**
   - 替换字符串处理为Protobuf处理
   - 实现帧协议的读写

### **验收标准**
- 编写Python测试客户端发送Protobuf消息
- 服务器正确解析并返回EchoResponse
- 无效数据不会导致程序崩溃

---

## **阶段 4: 消息路由系统**

### **目标**
建立可扩展的消息处理架构，支持多种消息类型的路由和处理。

### **实现内容**
1. **消息路由器** (`MessageRouter` class)
   - 基于消息类型分发到不同处理函数
   - 统一的错误处理和日志记录

2. **处理器接口** (`MessageHandler` interface)
   - 定义处理器的统一接口
   - 实现EchoHandler示例

3. **会话管理器** (`SessionManager` class)
   - 管理所有活跃Session
   - 提供Session查找功能
   - 优雅关闭和资源清理

### **验收标准**
- 消息正确路由到对应处理器
- 未知消息类型返回错误响应
- 可以轻松添加新的消息类型
- 代码结构清晰，易于扩展

---

## **阶段 5: 用户系统**

### **目标**
实现用户注册、登录和身份认证系统，集成MySQL数据库。

### **数据库设计**
```sql
-- 用户表
CREATE TABLE users (
    user_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_username (username)
);
```

### **实现内容**
1. **数据库管理** (`DatabaseManager` class)
   - MySQL连接管理
   - 统一的数据库操作接口

2. **用户管理** (`UserManager` class) 
   - 用户注册：用户名检查、密码哈希
   - 用户登录：密码验证、会话标记

3. **协议扩展**
   - 添加RegisterRequest/Response
   - 添加LoginRequest/Response

### **验收标准**
- 成功连接MySQL数据库
- 可以注册新用户，重复用户名被拒绝
- 可以登录已注册用户，错误密码被拒绝
- Python测试客户端验证完整流程

---


---

## **阶段 6: 私聊功能**

### **目标**
实现一对一消息发送、接收和历史记录查询功能。

### **数据库扩展**
```sql
-- 消息表
CREATE TABLE messages (
    message_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    from_user_id BIGINT NOT NULL,
    to_user_id BIGINT NOT NULL,
    content TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (from_user_id) REFERENCES users(user_id),
    FOREIGN KEY (to_user_id) REFERENCES users(user_id),
    INDEX idx_conversation (from_user_id, to_user_id, created_at)
);
```

### **实现内容**
1. **消息发送**
   - 验证登录状态和接收者存在
   - 消息存储到数据库
   - 在线用户实时推送

2. **消息接收**
   - 服务器主动推送MessagePush
   - 支持多用户同时在线

3. **历史记录**
   - 查询两用户间的聊天记录
   - 简单分页功能
   - 权限控制

### **验收标准**
- 登录用户可以发送和接收消息
- 所有消息正确存储
- 可以查看聊天历史
- Python客户端模拟完整聊天场景

---

## **阶段 7: 群聊功能**

### **目标**
实现基础的群组聊天功能，包括群组创建、成员管理和群消息广播。

### **数据库扩展**
```sql
-- 群组表
CREATE TABLE groups (
    group_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    group_name VARCHAR(100) NOT NULL,
    creator_user_id BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (creator_user_id) REFERENCES users(user_id)
);

-- 群成员表
CREATE TABLE group_members (
    group_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (group_id, user_id),
    FOREIGN KEY (group_id) REFERENCES groups(group_id),
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

-- 扩展消息表支持群聊
ALTER TABLE messages ADD COLUMN group_id BIGINT NULL;
ALTER TABLE messages ADD FOREIGN KEY (group_id) REFERENCES groups(group_id);
```

### **实现内容**
1. **群组管理**
   - 创建群组（CreateGroup）
   - 加入群组（JoinGroup）
   - 查看群组列表（ListGroups）

2. **群消息处理**
   - 群消息发送（SendGroupMessage）
   - 群消息广播给所有在线成员
   - 群消息历史查询

3. **协议扩展**
   ```protobuf
   message CreateGroupRequest {
       string group_name = 1;
   }
   
   message JoinGroupRequest {
       uint64 group_id = 1;
   }
   
   message SendGroupMessageRequest {
       uint64 group_id = 1;
       string content = 2;
   }
   ```

### **验收标准**
- 用户可以创建和加入群组
- 群消息正确广播给所有在线成员
- 群聊历史记录查询正常
- Python客户端模拟多用户群聊场景

---


---

## **完整功能演示流程**

### **端到端测试场景**
1. **用户注册**: 客户端A注册用户"alice"，客户端B注册用户"bob"
2. **用户登录**: 两个客户端分别登录各自账号
3. **发送消息**: alice向bob发送消息"Hello Bob!"
4. **实时接收**: bob立即收到来自alice的消息推送
5. **查看历史**: bob查询与alice的聊天历史，能看到刚才的消息

### **技术栈展示**
- **网络编程**: Asio异步TCP服务器
- **协议设计**: Protobuf消息定义和序列化
- **数据库操作**: MySQL存储用户和消息数据
- **安全处理**: bcrypt密码哈希
- **并发处理**: 多用户同时在线和消息收发
- **架构设计**: 分层的Server-Session-Handler架构

这个简化版本保持了核心功能的完整性，同时去除了不必要的复杂性，更适合作为简历项目展示技术能力。