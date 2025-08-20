# Dependency test executable
add_executable(dependency_test dependency_test.cpp)

# Link required libraries
target_link_libraries(dependency_test 
    spdlog::spdlog         # spdlog
    mysqlcppconn           # MySQL Connector/C++
    crypt                  # bcrypt/password hashing
    Threads::Threads       # threading support
)

# No additional include directories needed for spdlog target