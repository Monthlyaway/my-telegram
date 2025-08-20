# MyTelegram 项目架构框架图

本文档使用 Mermaid 图表全面展示 MyTelegram 即时通讯系统的架构设计，涵盖系统架构、技术栈、网络通信、数据库设计以及各种业务流程。

## 目录
- [MyTelegram 项目架构框架图](#mytelegram-项目架构框架图)
  - [目录](#目录)
  - [1. 总体系统架构](#1-总体系统架构)
  - [2. 技术栈架构](#2-技术栈架构)
  - [3. 网络通信架构](#3-网络通信架构)
  - [4. 服务器内部架构](#4-服务器内部架构)
  - [5. 协议层架构](#5-协议层架构)
  - [6. 数据库架构](#6-数据库架构)
    - [表结构说明:](#表结构说明)
    - [索引策略:](#索引策略)
  - [7. 消息路由流程](#7-消息路由流程)
  - [8. 用户认证流程](#8-用户认证流程)
  - [9. 私聊消息流程](#9-私聊消息流程)
  - [10. 群聊消息流程](#10-群聊消息流程)
  - [总结](#总结)
    - [🏗️ 架构亮点:](#️-架构亮点)
    - [🚀 技术特色:](#-技术特色)

---

## 1. 总体系统架构

展示整个系统的高级组件架构，包括客户端、服务器和数据存储层。

```mermaid
graph TB
    subgraph "客户端层"
        C1[Python测试客户端]
        C2[Telnet/Netcat工具]
        C3[未来的GUI客户端]
    end
    
    subgraph "网络层"
        TCP[TCP Socket连接\n端口: 8080]
        PROTO[Protobuf协议\n帧格式: 长度, 数据]
    end
    
    subgraph "应用服务器"
        SERVER[IM Server\nC++17 + Asio]
        
        subgraph "核心模块"
            SM[Session Manager\n连接管理]
            MR[Message Router\n消息路由]
            UM[User Manager\n用户管理]
            GM[Group Manager\n群组管理]
        end
    end
    
    subgraph "数据层"
        MYSQL[(MySQL 8.0+\nim_chat数据库)]
        CONFIG[JSON配置文件\nconfig.json]
        LOGS[日志文件\nspdlog输出]
    end
    
    C1 --> TCP
    C2 --> TCP
    C3 --> TCP
    
    TCP --> PROTO
    PROTO --> SERVER
    
    SERVER --> SM
    SERVER --> MR
    SERVER --> UM
    SERVER --> GM
    
    UM --> MYSQL
    GM --> MYSQL
    MR --> MYSQL
    
    SERVER --> CONFIG
    SERVER --> LOGS
    
    classDef client fill:#e1f5fe
    classDef server fill:#f3e5f5
    classDef data fill:#e8f5e8
    
    class C1,C2,C3 client
    class SERVER,SM,MR,UM,GM server
    class MYSQL,CONFIG,LOGS data

```

**架构说明:**
- **客户端层**: 支持多种客户端类型，通过TCP连接到服务器
- **网络层**: 使用TCP协议和自定义的Protobuf帧格式进行通信
- **应用服务器**: C++实现的高性能异步服务器，采用模块化设计
- **数据层**: MySQL数据库持久化存储，配置文件和日志支持

---

## 2. 技术栈架构

展示项目使用的完整技术栈和各层次的依赖关系。

```mermaid
graph TB
    subgraph "开发工具层"
        CMAKE[CMake 3.15+<br/>构建系统]
        GCC[GCC 13.3<br/>C++17编译器]
        GIT[Git版本控制]
    end
    
    subgraph "应用层"
        APP[IM Server Application]
    end
    
    subgraph "框架层"
        ASIO[Asio Standalone<br/>异步网络I/O]
        PROTOBUF[Google Protobuf<br/>消息序列化]
        SPDLOG[spdlog<br/>高性能日志]
        JSON[nlohmann/json<br/>JSON解析]
    end
    
    subgraph "系统库层"
        MYSQL[MySQL Connector/C++<br/>数据库驱动]
        CRYPT[libcrypt<br/>密码哈希]
        THREADS[std::thread<br/>并发支持]
        STL[C++ STL<br/>标准库]
    end
    
    subgraph "操作系统层"
        LINUX[Linux WSL2<br/>Ubuntu 22.04]
        NETWORK[TCP/IP网络栈]
    end
    
    subgraph "测试层"
        GTEST[Google Test<br/>单元测试]
        PYTHON[Python 3.8+<br/>集成测试]
    end
    
    CMAKE --> GCC
    CMAKE --> APP
    
    APP --> ASIO
    APP --> PROTOBUF
    APP --> SPDLOG
    APP --> JSON
    
    APP --> MYSQL
    APP --> CRYPT
    APP --> THREADS
    APP --> STL
    
    ASIO --> LINUX
    MYSQL --> LINUX
    NETWORK --> LINUX
    
    APP --> GTEST
    PYTHON --> APP
    
    classDef tools fill:#fff3e0
    classDef app fill:#f3e5f5
    classDef framework fill:#e8f5e8
    classDef system fill:#e1f5fe
    classDef os fill:#fce4ec
    classDef test fill:#f1f8e9
    
    class CMAKE,GCC,GIT tools
    class APP app
    class ASIO,PROTOBUF,SPDLOG,JSON framework
    class MYSQL,CRYPT,THREADS,STL system
    class LINUX,NETWORK os
    class GTEST,PYTHON test
```

**技术栈说明:**
- **现代C++**: 使用C++17特性，提供高性能和内存安全
- **异步I/O**: Asio库实现高并发网络处理
- **结构化通信**: Protobuf确保类型安全的消息传输
- **企业级数据库**: MySQL提供可靠的数据持久化
- **全面测试**: 单元测试+集成测试保证代码质量

---

## 3. 网络通信架构

展示TCP连接管理、会话处理和协议解析的网络层设计。

```mermaid
graph TB
    subgraph "客户端侧"
        CLIENT[客户端应用]
        SOCK_C[Socket连接]
    end
    
    subgraph "网络传输"
        TCP_CONN[TCP连接]
        FRAME[帧协议 4字节长度, Protobuf数据]
    end
    
    subgraph "服务器网络层"
        ACCEPTOR[Asio TCP Acceptor 监听新连接]
        
        subgraph "连接池"
            SESS1[Session 1 用户A]
            SESS2[Session 2 用户B]
            SESS3[Session N 用户...]
        end
        
        IO_CONTEXT[io_context 事件循环]
        THREAD_POOL[Worker Threads 4个工作线程]
    end
    
    subgraph "协议处理层"
        PROTO_HANDLER[ProtocolHandler 帧解析/序列化]
        MSG_VALIDATOR[消息验证 版本检查/格式校验]
    end
    
    CLIENT --> SOCK_C
    SOCK_C --> TCP_CONN
    TCP_CONN --> FRAME
    
    FRAME --> ACCEPTOR
    ACCEPTOR --> SESS1
    ACCEPTOR --> SESS2
    ACCEPTOR --> SESS3
    
    SESS1 --> IO_CONTEXT
    SESS2 --> IO_CONTEXT
    SESS3 --> IO_CONTEXT
    
    IO_CONTEXT --> THREAD_POOL
    
    SESS1 --> PROTO_HANDLER
    SESS2 --> PROTO_HANDLER
    SESS3 --> PROTO_HANDLER
    
    PROTO_HANDLER --> MSG_VALIDATOR
    
    classDef client fill:#e1f5fe
    classDef network fill:#fff3e0
    classDef server fill:#f3e5f5
    classDef protocol fill:#e8f5e8
    
    class CLIENT,SOCK_C client
    class TCP_CONN,FRAME network
    class ACCEPTOR,SESS1,SESS2,SESS3,IO_CONTEXT,THREAD_POOL server
    class PROTO_HANDLER,MSG_VALIDATOR protocol

```

**网络架构特点:**
- **异步I/O模型**: 使用Asio库实现高并发连接处理
- **会话管理**: 每个客户端连接对应一个Session对象
- **事件驱动**: io_context配合线程池处理网络事件
- **协议安全**: 帧长度限制(1MB)防止内存攻击

---

## 4. 服务器内部架构

展示服务器核心类的关系和消息处理的内部流程。

```mermaid
graph TB
    subgraph "服务器主类"
        MAIN[main.cpp<br/>程序入口]
        SERVER[Server类<br/>服务器核心]
        CONFIG_MGR[ConfigManager<br/>配置管理]
    end
    
    subgraph "会话管理层"
        SESSION_MGR[SessionManager<br/>会话管理器]
        
        subgraph "会话实例"
            SESSION1[Session<br/>用户连接1]
            SESSION2[Session<br/>用户连接2]
            SESSIONN[Session<br/>用户连接N]
        end
    end
    
    subgraph "消息处理层"
        MSG_ROUTER[MessageRouter<br/>消息路由器]
        
        subgraph "消息处理器"
            ECHO_HANDLER[EchoHandler<br/>回声处理]
            USER_HANDLER[UserHandler<br/>用户管理]
            MSG_HANDLER[MessageHandler<br/>消息处理]
            GROUP_HANDLER[GroupHandler<br/>群组处理]
        end
    end
    
    subgraph "业务逻辑层"
        USER_MGR[UserManager<br/>用户业务逻辑]
        GROUP_MGR[GroupManager<br/>群组业务逻辑]
        AUTH_MGR[AuthManager<br/>认证管理]
    end
    
    subgraph "数据访问层"
        DB_MGR[DatabaseManager<br/>数据库管理]
        CONN_POOL[Connection Pool<br/>连接池]
        
        subgraph "DAO层"
            USER_DAO[UserDAO<br/>用户数据访问]
            MSG_DAO[MessageDAO<br/>消息数据访问]
            GROUP_DAO[GroupDAO<br/>群组数据访问]
        end
    end
    
    MAIN --> SERVER
    MAIN --> CONFIG_MGR
    
    SERVER --> SESSION_MGR
    SESSION_MGR --> SESSION1
    SESSION_MGR --> SESSION2
    SESSION_MGR --> SESSIONN
    
    SESSION1 --> MSG_ROUTER
    SESSION2 --> MSG_ROUTER
    SESSIONN --> MSG_ROUTER
    
    MSG_ROUTER --> ECHO_HANDLER
    MSG_ROUTER --> USER_HANDLER
    MSG_ROUTER --> MSG_HANDLER
    MSG_ROUTER --> GROUP_HANDLER
    
    USER_HANDLER --> USER_MGR
    MSG_HANDLER --> USER_MGR
    GROUP_HANDLER --> GROUP_MGR
    USER_HANDLER --> AUTH_MGR
    
    USER_MGR --> DB_MGR
    GROUP_MGR --> DB_MGR
    AUTH_MGR --> DB_MGR
    
    DB_MGR --> CONN_POOL
    DB_MGR --> USER_DAO
    DB_MGR --> MSG_DAO
    DB_MGR --> GROUP_DAO
    
    classDef main fill:#ffcdd2
    classDef session fill:#f3e5f5
    classDef message fill:#e8f5e8
    classDef business fill:#fff3e0
    classDef data fill:#e1f5fe
    
    class MAIN,SERVER,CONFIG_MGR main
    class SESSION_MGR,SESSION1,SESSION2,SESSIONN session
    class MSG_ROUTER,ECHO_HANDLER,USER_HANDLER,MSG_HANDLER,GROUP_HANDLER message
    class USER_MGR,GROUP_MGR,AUTH_MGR business
    class DB_MGR,CONN_POOL,USER_DAO,MSG_DAO,GROUP_DAO data
```

**架构设计原则:**
- **分层架构**: 清晰的职责分离，从网络层到数据层
- **依赖注入**: 上层依赖接口，下层实现具体功能
- **单一职责**: 每个类专注于特定的功能领域
- **可扩展性**: 新的消息类型只需添加对应的Handler

---

## 5. 协议层架构

展示Protobuf消息格式、帧结构和协议处理的详细设计。

```mermaid
graph TB
    subgraph "协议格式"
        FRAME_FORMAT[帧格式\n4字节长度\nProtobuf数据]
        MAX_SIZE[最大帧长度 1MB\n防止内存攻击]
        BYTE_ORDER[网络字节序\n大端格式]
    end
    
    subgraph "Protobuf消息结构"
        PACKET[Packet 根消息]
        VERSION[version uint32\n协议版本=1]
        SEQUENCE[sequence uint32\n请求-响应匹配]
        
        subgraph "系统消息 (1-99)"
            ECHO_REQ[EchoRequest = 10\nEchoResponse = 11]
            HEARTBEAT[HeartbeatRequest = 20\nHeartbeatResponse = 21]
        end
        
        subgraph "用户管理 (100-199)"
            REGISTER[RegisterRequest = 100\nRegisterResponse = 101]
            LOGIN[LoginRequest = 102\nLoginResponse = 103]
        end
        
        subgraph "消息相关 (200-299)"
            SEND_MSG[SendMessageRequest = 200\nSendMessageResponse = 201]
            MSG_PUSH[MessagePush = 202]
        end
        
        subgraph "群组功能 (300-399)"
            CREATE_GROUP[CreateGroupRequest = 300\nCreateGroupResponse = 301]
            JOIN_GROUP[JoinGroupRequest = 302\nJoinGroupResponse = 303]
            SEND_GROUP_MSG[SendGroupMessageRequest = 304]
        end
        
        ERROR_RESP[ErrorResponse = 999\n统一错误处理]
    end
    
    subgraph "错误处理体系"
        ERROR_CODES[错误码分类]
        PROTO_ERR[1000-1099\n协议错误\n版本不匹配/格式错误]
        AUTH_ERR[2000-2099\n认证错误\n登录失败/会话过期]
        BIZ_ERR[3000-3099\n业务错误\n用户不存在/权限不足]
        SYS_ERR[4000-4099\n系统错误\n数据库错误/内部异常]
    end
    
    FRAME_FORMAT --> PACKET
    MAX_SIZE --> PACKET
    BYTE_ORDER --> PACKET
    
    PACKET --> VERSION
    PACKET --> SEQUENCE
    PACKET --> ECHO_REQ
    PACKET --> HEARTBEAT
    PACKET --> REGISTER
    PACKET --> LOGIN
    PACKET --> SEND_MSG
    PACKET --> MSG_PUSH
    PACKET --> CREATE_GROUP
    PACKET --> JOIN_GROUP
    PACKET --> SEND_GROUP_MSG
    PACKET --> ERROR_RESP
    
    ERROR_RESP --> ERROR_CODES
    ERROR_CODES --> PROTO_ERR
    ERROR_CODES --> AUTH_ERR
    ERROR_CODES --> BIZ_ERR
    ERROR_CODES --> SYS_ERR
    
    classDef frame fill:#f3e5f5
    classDef message fill:#e8f5e8
    classDef system fill:#fff3e0
    classDef user fill:#e1f5fe
    classDef chat fill:#f1f8e9
    classDef group fill:#fce4ec
    classDef error fill:#ffebee
    
    class FRAME_FORMAT,MAX_SIZE,BYTE_ORDER frame
    class PACKET,VERSION,SEQUENCE message
    class ECHO_REQ,HEARTBEAT system
    class REGISTER,LOGIN user
    class SEND_MSG,MSG_PUSH chat
    class CREATE_GROUP,JOIN_GROUP,SEND_GROUP_MSG group
    class ERROR_RESP,ERROR_CODES,PROTO_ERR,AUTH_ERR,BIZ_ERR,SYS_ERR error

```

**协议设计特点:**
- **版本兼容**: 协议版本字段支持向后兼容演进
- **类型安全**: Protobuf强类型系统确保消息安全
- **可扩展性**: 预留字段编号便于功能扩展
- **标准化错误**: 统一的错误码体系和处理机制

---

## 6. 数据库架构

展示MySQL数据库的表结构设计和关系模型。

```mermaid
erDiagram
    USERS {
        bigint user_id PK
        varchar username UK
        varchar password_hash
        timestamp created_at
    }
    
    MESSAGES {
        bigint message_id PK
        bigint from_user_id FK
        bigint to_user_id FK
        bigint group_id FK
        text content
        timestamp created_at
    }
    
    GROUPS {
        bigint group_id PK
        varchar group_name
        bigint creator_user_id FK
        timestamp created_at
    }
    
    GROUP_MEMBERS {
        bigint group_id PK,FK
        bigint user_id PK,FK
        timestamp joined_at
    }
    
    SESSIONS {
        varchar session_id PK
        bigint user_id FK
        varchar ip_address
        timestamp created_at
        timestamp last_active
    }
    
    %% 关系定义
    USERS ||--o{ MESSAGES : "from_user_id"
    USERS ||--o{ MESSAGES : "to_user_id"
    USERS ||--o{ GROUPS : "creator_user_id"
    USERS ||--o{ GROUP_MEMBERS : "user_id"
    USERS ||--o{ SESSIONS : "user_id"
    
    GROUPS ||--o{ MESSAGES : "group_id"
    GROUPS ||--o{ GROUP_MEMBERS : "group_id"
```

**数据库设计特点:**

### 表结构说明:

1. **USERS 用户表**
   - `user_id`: 主键，自增ID
   - `username`: 唯一用户名，建立索引
   - `password_hash`: bcrypt哈希密码
   - `created_at`: 注册时间

2. **MESSAGES 消息表**
   - `message_id`: 主键，自增ID
   - `from_user_id/to_user_id`: 私聊消息的发送方和接收方
   - `group_id`: 群聊消息的群组ID（可为空）
   - `content`: 消息内容
   - `created_at`: 消息时间
   - 索引: `(from_user_id, to_user_id, created_at)` 支持会话历史查询

3. **GROUPS 群组表**
   - `group_id`: 主键，自增ID
   - `group_name`: 群组名称
   - `creator_user_id`: 群组创建者
   - `created_at`: 创建时间

4. **GROUP_MEMBERS 群成员表**
   - 复合主键: `(group_id, user_id)`
   - `joined_at`: 加入时间
   - 支持多对多关系：用户-群组

5. **SESSIONS 会话表**
   - `session_id`: 会话唯一标识
   - `user_id`: 关联用户
   - `ip_address`: 客户端IP
   - `last_active`: 最后活跃时间

### 索引策略:
- `users.username`: 唯一索引，登录查询
- `messages.conversation`: 复合索引，聊天历史查询
- `group_members`: 复合主键，群成员关系查询
- `sessions.user_id`: 普通索引，用户会话查询

---

## 7. 消息路由流程

展示从接收客户端消息到业务处理完成的完整流程。

```mermaid
sequenceDiagram
    participant Client as 客户端
    participant Session as Session会话
    participant Router as MessageRouter
    participant Handler as 消息处理器
    participant Manager as 业务管理器
    participant Database as 数据库
    participant OtherSession as 其他Session
    
    Client->>+Session: TCP消息 [长度][Protobuf数据]
    Session->>Session: 解析帧格式
    Session->>Session: 反序列化Protobuf
    Session->>Session: 验证协议版本
    
    Session->>+Router: route(Packet消息)
    Router->>Router: 解析消息类型
    Router->>+Handler: handle(具体消息类型)
    
    Handler->>Handler: 验证用户权限
    Handler->>Handler: 参数校验
    Handler->>+Manager: 调用业务逻辑
    
    Manager->>+Database: 数据库操作
    Database-->>-Manager: 操作结果
    
    Manager->>Manager: 业务逻辑处理
    Manager-->>-Handler: 处理结果
    
    Handler->>Handler: 构造响应消息
    Handler-->>-Router: Response消息
    
    Router-->>-Session: 路由响应
    Session->>Session: 序列化Protobuf
    Session->>Session: 封装帧格式
    Session-->>-Client: TCP响应消息
    
    note over Handler,Manager: 如果需要推送给其他用户
    Handler->>+OtherSession: 消息推送
    OtherSession->>OtherSession: 序列化推送消息
    OtherSession-->>-Client: 主动推送
```

**流程关键点:**
1. **帧解析**: 验证长度字段，防止缓冲区溢出
2. **协议验证**: 检查版本兼容性和消息格式
3. **权限检查**: 验证用户登录状态和操作权限  
4. **事务处理**: 数据库操作的原子性保证
5. **异步推送**: 支持向其他在线用户主动推送消息

---

## 8. 用户认证流程

展示用户注册和登录的完整业务流程。

```mermaid
graph TD
    subgraph "用户注册流程"
        REG_START[客户端发送注册请求]
        REG_VALIDATE[验证用户名格式]
        REG_CHECK[检查用户名是否存在]
        REG_HASH[bcrypt密码哈希]
        REG_SAVE[保存到数据库]
        REG_RESPONSE[返回注册结果]
        
        REG_START --> REG_VALIDATE
        REG_VALIDATE --> REG_CHECK
        REG_CHECK -->|用户名可用| REG_HASH
        REG_CHECK -->|用户名已存在| REG_RESPONSE
        REG_HASH --> REG_SAVE
        REG_SAVE --> REG_RESPONSE
    end
    
    subgraph "用户登录流程"
        LOGIN_START[客户端发送登录请求]
        LOGIN_QUERY[查询用户信息]
        LOGIN_VERIFY[验证密码哈希]
        LOGIN_SESSION[创建会话记录]
        LOGIN_MARK[标记Session已认证]
        LOGIN_RESPONSE[返回登录结果]
        
        LOGIN_START --> LOGIN_QUERY
        LOGIN_QUERY -->|用户存在| LOGIN_VERIFY
        LOGIN_QUERY -->|用户不存在| LOGIN_RESPONSE
        LOGIN_VERIFY -->|密码正确| LOGIN_SESSION
        LOGIN_VERIFY -->|密码错误| LOGIN_RESPONSE
        LOGIN_SESSION --> LOGIN_MARK
        LOGIN_MARK --> LOGIN_RESPONSE
    end
    
    subgraph "会话管理"
        SESSION_TIMEOUT[会话超时检查]
        SESSION_CLEANUP[清理过期会话]
        HEARTBEAT_CHECK[心跳检测]
        
        SESSION_TIMEOUT --> SESSION_CLEANUP
        HEARTBEAT_CHECK --> SESSION_TIMEOUT
    end
    
    REG_RESPONSE -.-> LOGIN_START
    LOGIN_MARK -.-> HEARTBEAT_CHECK
    
    classDef register fill:#e8f5e8
    classDef login fill:#e1f5fe
    classDef session fill:#fff3e0
    
    class REG_START,REG_VALIDATE,REG_CHECK,REG_HASH,REG_SAVE,REG_RESPONSE register
    class LOGIN_START,LOGIN_QUERY,LOGIN_VERIFY,LOGIN_SESSION,LOGIN_MARK,LOGIN_RESPONSE login
    class SESSION_TIMEOUT,SESSION_CLEANUP,HEARTBEAT_CHECK session
```

**安全特性:**
- **密码安全**: 使用bcrypt算法，加盐哈希存储
- **会话管理**: 基于时间的会话超时机制
- **重复登录**: 支持同一用户多设备登录
- **心跳检测**: 定期检测连接状态，自动清理僵尸会话

---

## 9. 私聊消息流程

展示一对一消息发送、存储和实时推送的完整流程。

```mermaid
sequenceDiagram
    participant Alice as Alice客户端
    participant AliceSession as Alice Session
    participant Server as 服务器
    participant Database as 数据库
    participant BobSession as Bob Session
    participant Bob as Bob客户端
    
    Alice->>+AliceSession: SendMessageRequest
    note over Alice,AliceSession: 消息内容: "Hello Bob!"<br/>接收者: bob
    
    AliceSession->>+Server: 路由消息到MessageHandler
    Server->>Server: 验证Alice已登录
    Server->>+Database: 查询接收者Bob是否存在
    Database-->>-Server: 用户存在，获取user_id
    
    Server->>+Database: 保存消息到messages表
    note over Database: INSERT INTO messages<br/>(from_user_id, to_user_id, content)
    Database-->>-Server: 消息保存成功，获取message_id
    
    Server-->>-AliceSession: SendMessageResponse(成功)
    AliceSession-->>-Alice: 消息发送确认
    
    note over Server: 检查Bob是否在线
    Server->>+BobSession: 查找Bob的活跃Session
    BobSession->>BobSession: 构造MessagePush消息
    note over BobSession: MessagePush {<br/>  from_username: "alice"<br/>  content: "Hello Bob!"<br/>  timestamp: now<br/>}
    
    BobSession-->>-Bob: 实时推送新消息
    Bob->>Bob: 显示新消息通知
    
    note over Alice,Bob: 如果Bob离线，消息仍保存在数据库<br/>Bob下次登录时可查询历史消息
```

**私聊特性:**
- **消息持久化**: 所有消息存储到数据库，支持历史查询
- **实时推送**: 在线用户立即收到新消息通知
- **离线支持**: 用户离线时消息保存，上线后可查询
- **消息确认**: 发送方收到消息投递确认

---

## 10. 群聊消息流程

展示群组消息的广播分发和成员管理流程。

```mermaid
sequenceDiagram
    participant Alice as Alice客户端
    participant AliceSession as Alice Session
    participant Server as 服务器
    participant Database as 数据库
    participant BobSession as Bob Session
    participant Bob as Bob客户端
    participant CharlieSession as Charlie Session
    participant Charlie as Charlie客户端
    
    note over Alice,Charlie: 群组"开发讨论组"已创建，Alice、Bob、Charlie都是成员
    
    Alice->>+AliceSession: SendGroupMessageRequest
    note over Alice,AliceSession: 消息内容: "今天的代码评审开始"<br/>群组ID: 1001
    
    AliceSession->>+Server: 路由到GroupHandler
    Server->>+Database: 验证Alice是群组成员
    Database-->>-Server: 验证通过
    
    Server->>+Database: 保存群组消息
    note over Database: INSERT INTO messages<br/>(from_user_id, group_id, content)
    Database-->>-Server: 消息保存成功
    
    Server->>+Database: 查询群组所有成员
    Database-->>-Server: 返回成员列表 [Alice, Bob, Charlie]
    
    Server-->>-AliceSession: SendGroupMessageResponse(成功)
    AliceSession-->>-Alice: 群消息发送确认
    
    note over Server: 向所有在线群成员广播
    
    par 并行推送给Bob
        Server->>+BobSession: 查找Bob Session
        BobSession->>BobSession: 构造GroupMessagePush
        BobSession-->>-Bob: 推送群消息
        Bob->>Bob: 显示群组新消息
    and 并行推送给Charlie  
        Server->>+CharlieSession: 查找Charlie Session
        CharlieSession->>CharlieSession: 构造GroupMessagePush
        CharlieSession-->>-Charlie: 推送群消息
        Charlie->>Charlie: 显示群组新消息
    and 不推送给Alice
        note over Server,AliceSession: 发送者无需推送给自己
    end
    
    note over Alice,Charlie: 如果某成员离线，消息仍保存<br/>该成员上线后可查询群聊历史
```

**群聊特性:**
- **成员验证**: 只有群组成员才能发送群消息
- **广播机制**: 消息自动推送给所有在线群成员  
- **发送者排除**: 避免向消息发送者推送自己的消息
- **并行处理**: 使用异步I/O同时向多个成员推送
- **历史保存**: 群组消息持久化，支持历史记录查询

---

## 总结

MyTelegram项目采用现代C++和成熟的开源技术栈，实现了高性能、可扩展的即时通讯系统。整体架构特点：

### 🏗️ 架构亮点:
- **分层设计**: 网络层→协议层→业务层→数据层，职责清晰
- **异步I/O**: 基于Asio的高并发网络处理
- **消息路由**: 灵活的消息类型分发和处理机制
- **协议安全**: Protobuf类型安全+帧长度限制
- **数据一致性**: MySQL事务支持和规范化设计

### 🚀 技术特色:
- **现代C++17**: 智能指针、异步编程、标准库特性
- **企业级组件**: MySQL数据库、spdlog日志、bcrypt安全
- **完整测试**: 单元测试+集成测试+性能测试
- **易于扩展**: 插件式Handler架构，新功能快速集成
- **生产就绪**: 配置管理、错误处理、监控日志完备

这个架构设计既保证了系统的高性能和稳定性，又为后续功能扩展提供了良好的基础。