cmake_minimum_required(VERSION 3.10)
project(MyTelegram CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Find spdlog
find_package(spdlog REQUIRED)

# MySQL Connector/C++ include path (based on your working example)
include_directories(/usr/include/mysql-cppconn)

# Asio is header-only, typically in /usr/include/asio.hpp
# nlohmann/json is header-only, typically in /usr/include/nlohmann/json.hpp

# Source files for the main server
set(SERVER_SOURCES
    src/main.cpp
    src/config/config.cpp
    src/server/server.cpp
    src/server/session.cpp
)

# Main server executable
add_executable(im_server ${SERVER_SOURCES})

# Include directories
target_include_directories(im_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link required libraries
target_link_libraries(im_server 
    spdlog::spdlog         # spdlog
    mysqlcppconn           # MySQL Connector/C++
    crypt                  # bcrypt/password hashing
    Threads::Threads       # threading support
)

# Add tests subdirectory
add_subdirectory(tests)