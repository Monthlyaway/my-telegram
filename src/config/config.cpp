#include "config.h"
#include <fstream>
#include <iostream>

bool Config::load_from_file(const std::string& config_path) {
    try {
        std::ifstream config_file(config_path);
        if (!config_file.is_open()) {
            std::cerr << "Failed to open config file: " << config_path << std::endl;
            return false;
        }

        nlohmann::json j;
        config_file >> j;

        // Parse server config
        const auto& server_json = j["server"];
        server_.host = server_json["host"];
        server_.port = server_json["port"];
        server_.max_connections = server_json["max_connections"];
        server_.worker_threads = server_json["worker_threads"];

        // Parse logging config
        const auto& logging_json = j["logging"];
        logging_.level = logging_json["level"];
        logging_.file = logging_json["file"];
        logging_.max_size_mb = logging_json["max_size_mb"];
        logging_.max_files = logging_json["max_files"];

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse config file: " << e.what() << std::endl;
        return false;
    }
}