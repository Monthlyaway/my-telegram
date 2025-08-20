#include <iostream>
#include <stdexcept>
#include <memory>
#include <string>

// Test all required dependencies

// 1. Asio (standalone version)
#define ASIO_STANDALONE
#include <asio.hpp>

// 2. spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// 3. nlohmann/json
#include <nlohmann/json.hpp>

// 4. bcrypt/crypt
#include <crypt.h>

// 5. MySQL Connector/C++
#include <mysql/jdbc.h>

using json = nlohmann::json;

bool test_asio()
{
    try
    {
        std::cout << "Testing Asio..." << std::endl;

        // Create io_context
        asio::io_context io_context;

        // Create a simple timer to test Asio functionality
        asio::steady_timer timer(io_context, asio::chrono::seconds(0));
        timer.wait();

        std::cout << "✅ Asio: OK" << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "❌ Asio: FAILED - " << e.what() << std::endl;
        return false;
    }
}

bool test_spdlog()
{
    try
    {
        std::cout << "Testing spdlog..." << std::endl;

        // Create a console logger
        auto console = spdlog::stdout_color_mt("console");
        console->info("spdlog test message");
        console->set_level(spdlog::level::debug);

        // Test different log levels
        console->debug("Debug message");
        console->warn("Warning message");

        std::cout << "✅ spdlog: OK" << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "❌ spdlog: FAILED - " << e.what() << std::endl;
        return false;
    }
}

bool test_json()
{
    try
    {
        std::cout << "Testing nlohmann/json..." << std::endl;

        // Create JSON object
        json j;
        j["name"] = "test";
        j["version"] = 1;
        j["settings"] = {
            {"host", "localhost"},
            {"port", 8080}};

        // Test serialization
        std::string json_str = j.dump();

        // Test parsing
        json parsed = json::parse(json_str);

        if (parsed["name"] == "test" && parsed["version"] == 1)
        {
            std::cout << "✅ nlohmann/json: OK" << std::endl;
            return true;
        }
        else
        {
            std::cout << "❌ nlohmann/json: FAILED - parsing error" << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "❌ nlohmann/json: FAILED - " << e.what() << std::endl;
        return false;
    }
}

bool test_bcrypt()
{
    try
    {
        std::cout << "Testing bcrypt/crypt..." << std::endl;

        const char *password = "test123";
        const char *salt = "$6$rounds=5000$testsalt$";

        // Hash the password
        char *hashed = crypt(password, salt);

        if (hashed != nullptr)
        {
            // Verify the hash
            char *verified = crypt(password, hashed);
            if (verified != nullptr && std::string(hashed) == std::string(verified))
            {
                std::cout << "✅ bcrypt/crypt: OK" << std::endl;
                return true;
            }
            else
            {
                std::cout << "❌ bcrypt/crypt: FAILED - verification failed" << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << "❌ bcrypt/crypt: FAILED - hashing failed" << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "❌ bcrypt/crypt: FAILED - " << e.what() << std::endl;
        return false;
    }
}

bool test_mysql()
{
    try
    {
        std::cout << "Testing MySQL Connector/C++..." << std::endl;

        sql::Driver *driver = get_driver_instance();

        // Connect to database (using your provided credentials)
        std::unique_ptr<sql::Connection> con(
            driver->connect("tcp://127.0.0.1:3306", "will", "abcd1234"));

        std::cout << "  MySQL connection established" << std::endl;

        // Select database
        con->setSchema("testdb");

        // Execute test SQL
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS dependency_test(id INT PRIMARY KEY AUTO_INCREMENT, msg VARCHAR(50))");
        stmt->execute("INSERT INTO dependency_test(msg) VALUES('Dependency test successful')");

        // Query data to verify
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM dependency_test LIMIT 1"));
        if (res->next())
        {
            std::cout << "  Test record: id=" << res->getInt("id")
                      << ", msg=" << res->getString("msg") << std::endl;
        }

        std::cout << "✅ MySQL Connector/C++: OK" << std::endl;
        return true;
    }
    catch (sql::SQLException &e)
    {
        std::cout << "❌ MySQL Connector/C++: FAILED - " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << ", SQLState: " << e.getSQLState() << ")" << std::endl;
        return false;
    }
    catch (const std::exception &e)
    {
        std::cout << "❌ MySQL Connector/C++: FAILED - " << e.what() << std::endl;
        return false;
    }
}

int main()
{
    std::cout << "=== Dependency Test Program ===" << std::endl;
    std::cout << "Testing all required libraries for MyTelegram project" << std::endl;
    std::cout << std::endl;

    int failed_tests = 0;

    // Test all dependencies
    if (!test_asio())
        failed_tests++;
    if (!test_spdlog())
        failed_tests++;
    if (!test_json())
        failed_tests++;
    if (!test_bcrypt())
        failed_tests++;
    if (!test_mysql())
        failed_tests++;

    std::cout << std::endl;
    std::cout << "=== Test Results ===" << std::endl;

    if (failed_tests == 0)
    {
        std::cout << "🎉 All dependencies are working correctly!" << std::endl;
        std::cout << "✅ Project is ready for development" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "❌ " << failed_tests << " dependencies failed!" << std::endl;
        std::cout << "Please install missing dependencies before continuing" << std::endl;
        return 1;
    }
}