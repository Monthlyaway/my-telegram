#pragma once

#include <string>
#include <nlohmann/json.hpp>

class Config {
public:
    struct ServerConfig {
        std::string host;
        int port;
        int max_connections;
        int worker_threads;
    };

    struct LoggingConfig {
        std::string level;
        std::string file;
        int max_size_mb;
        int max_files;
    };

    Config() = default;
    ~Config() = default;

    bool load_from_file(const std::string& config_path);
    
    const ServerConfig& get_server_config() const { return server_; }
    const LoggingConfig& get_logging_config() const { return logging_; }

private:
    ServerConfig server_;
    LoggingConfig logging_;
};