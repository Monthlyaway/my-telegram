# 指定 CMake 的最低版本要求为 3.10，如果低于这个版本，CMake 会报错
cmake_minimum_required(VERSION 3.10)

# 定义工程名为 MyTelegram，语言为 C++（CXX）
project(MyTelegram CXX)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
# 强制要求使用 C++17，如果编译器不支持，会报错
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 pkg-config 工具，它用于检查系统中库的安装路径和编译参数
find_package(PkgConfig REQUIRED)

# 查找线程库，一般用于 std::thread 或 pthread
find_package(Threads REQUIRED)

# 查找 protobuf 库，用于序列化/反序列化 .proto 文件定义的消息
find_package(Protobuf REQUIRED)

# 查找 spdlog 库，用于高性能日志打印
find_package(spdlog REQUIRED)

# 设置 MySQL Connector/C++ 的头文件路径
# 这样在 include 头文件时，编译器能找到 mysqlcppconn 的接口
include_directories(/usr/include/mysql-cppconn)

# 注释说明：
# Asio 是 header-only 的网络库，通常在 /usr/include/asio.hpp
# nlohmann/json 是 header-only 的 JSON 库，通常在 /usr/include/nlohmann/json.hpp
# 这里不需要显式链接库，只需要 include 头文件即可

# 使用 CMake 自带的 protobuf 功能，将 messages.proto 文件生成 C++ 源文件和头文件
# PROTO_SRCS 是生成的 .cpp 文件，PROTO_HDRS 是生成的 .h 文件, pb是protocol buffer的意思
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS protos/messages.proto)

# 定义服务器源码文件列表，包括 main.cpp、配置模块、服务器模块、会话模块、协议处理模块、路由模块，以及生成的 protobuf 文件
set(SERVER_SOURCES
    src/main.cpp
    src/config/config.cpp
    src/server/server.cpp
    src/server/session.cpp
    src/server/session_manager.cpp
    src/protocol/protocol_handler.cpp
    src/router/message_router.cpp
    src/router/message_handler.cpp
    ${PROTO_SRCS}  # protobuf 生成的源文件
)

# 创建一个可执行文件 im_server，使用上面定义的 SERVER_SOURCES
add_executable(im_server ${SERVER_SOURCES})

# 指定可执行文件的 include 目录
target_include_directories(im_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src         # 自己写的源码头文件
    ${CMAKE_CURRENT_BINARY_DIR}             # protobuf 生成的文件所在目录
    ${Protobuf_INCLUDE_DIRS}                # protobuf 库的头文件
)

# 链接所需库
target_link_libraries(im_server 
    spdlog::spdlog         # 日志库
    ${Protobuf_LIBRARIES}  # protobuf 库
    mysqlcppconn           # MySQL Connector/C++，用于数据库连接
    crypt                  # bcrypt 或密码哈希相关库
    Threads::Threads       # 线程支持库（pthread 或 Windows 线程库）
)

# 添加 tests 子目录，通常里面也有一个 CMakeLists.txt 用于构建测试程序
add_subdirectory(tests)
