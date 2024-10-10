#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <mutex>
#include <memory>
#include <nlohmann/json.hpp>

// Enumeration for application modes
enum class AppMode
{
    CLIENT,
    SERVER,
    NONE
};

// Structure to hold configuration settings
struct Configuration
{
    // Common settings
    bool use_web_server = true;
    unsigned short web_server_port = 15881;
    bool web_server_secure = false;
    std::string web_server_cert_path;
    std::string web_server_key_path;
    int threads = 10;

    // Client-specific settings
    bool use_client = false;
    unsigned short main_server_port = 15880;
    std::string client_server_ip;
    bool use_airplay = false;
    bool use_bluetooth = false;
    const char *spiDevicePath = "/dev/spidev0.0";
    const char *i2cDevicePath = "/dev/i2c-1";
    uint8_t i2cDeviceAddress = 0x3b;
    uint8_t micCount = 4;
    uint8_t ledCount = 16;
    bool setbluetooth = false;
    bool use_airplay = false;
    char *main_server_ip;

    // Server-specific settings
    bool use_server = false;
    bool use_terminal_input = false;
    bool use_homeassistant = false;
    std::string home_assistant_ip;
    std::string home_assistant_token;
    int home_assistant_port = 0;

    // Method to convert AppMode to string
    std::string get_mode_string() const
    {
        if (use_client)
            return "CLIENT";
        if (use_server)
            return "SERVER";
        return "NONE";
    }

    // Serialize Configuration to JSON
    nlohmann::json to_json() const;

    // Deserialize Configuration from JSON
    void from_json(const nlohmann::json &j);
};

// Singleton class to manage configuration
class ConfigurationManager
{
public:
    static ConfigurationManager &getInstance()
    {
        static ConfigurationManager instance;
        return instance;
    }

    // Get a copy of the current configuration
    Configuration getConfiguration()
    {
        std::lock_guard<std::mutex> lock(config_mutex);
        return config;
    }

    // Update configuration with a new config
    void updateConfiguration(const Configuration &new_config)
    {
        std::lock_guard<std::mutex> lock(config_mutex);
        config = new_config;
    }

    // Save configuration to a JSON file
    void saveConfiguration(const std::string &filepath);

    // Load configuration from a JSON file
    void loadConfiguration(const std::string &filepath);

    // Delete copy constructor and assignment operator
    ConfigurationManager(const ConfigurationManager &) = delete;
    ConfigurationManager &operator=(const ConfigurationManager &) = delete;

private:
    Configuration config;
    std::mutex config_mutex;

    // Private constructor for singleton
    ConfigurationManager() {}
};

#endif // CONFIGURATION_H
