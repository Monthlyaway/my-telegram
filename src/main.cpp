#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <signal.h>
#include <atomic>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "config/config.h"
#include "server/server.h"

std::unique_ptr<Server> g_server;
std::atomic<bool> g_running{true};

void signal_handler(int signal)
{
    spdlog::info("Received signal {}, shutting down gracefully...", signal);
    if (g_server)
    {
        g_server->stop();
    }
    g_running = false;
}

void setup_logging(const Config::LoggingConfig &logging_config)
{
    try
    {
        // Create console sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        // Create file sink
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logging_config.file,
            logging_config.max_size_mb * 1024 * 1024,
            logging_config.max_files);

        // Combine sinks
        std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};
        auto combined_logger = std::make_shared<spdlog::logger>("combined", sinks.begin(), sinks.end());

        // Set as default logger
        spdlog::set_default_logger(combined_logger);

        // Set log level
        if (logging_config.level == "debug")
        {
            spdlog::set_level(spdlog::level::debug);
        }
        else if (logging_config.level == "info")
        {
            spdlog::set_level(spdlog::level::info);
        }
        else if (logging_config.level == "warn")
        {
            spdlog::set_level(spdlog::level::warn);
        }
        else if (logging_config.level == "error")
        {
            spdlog::set_level(spdlog::level::err);
        }

        // Set pattern
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        spdlog::info("Logging initialized successfully");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to setup logging: " << e.what() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        // Setup signal handlers
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);

        // Load configuration
        Config config;
        std::string config_path = "config.json";

        if (argc > 1)
        {
            config_path = argv[1];
        }

        if (!config.load_from_file(config_path))
        {
            std::cerr << "Failed to load configuration from: " << config_path << std::endl;
            return 1;
        }

        // Setup logging
        setup_logging(config.get_logging_config());

        spdlog::info("=== MyTelegram IM Server ===");
        spdlog::info("Starting MyTelegram IM Server (Stage 2: Basic TCP Echo Server)");

        // Create and start server
        g_server = std::make_unique<Server>(config);

        if (!g_server->start())
        {
            spdlog::error("Failed to start server");
            return 1;
        }

        spdlog::info("Server started successfully. Press Ctrl+C to stop.");

        // Keep main thread running
        while (g_running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("Server error: {}", e.what());
        return 1;
    }

    return 0;
}