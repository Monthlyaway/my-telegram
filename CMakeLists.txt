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

# Add tests subdirectory
add_subdirectory(tests)