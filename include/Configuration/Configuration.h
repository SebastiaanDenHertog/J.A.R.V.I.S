#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <mutex>
#include <memory>
#include <unordered_map>
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
    bool use_bluetooth = false;
    int threads = 10;

    // Client-specific settings
    unsigned short main_server_port = 15880;
    std::string client_server_ip;
    bool use_airplay = false;
    bool use_client_server_connection = false;
    const char *spiDevicePath = "/dev/spidev0.0";
    const char *i2cDevicePath = "/dev/i2c-1";
    uint8_t i2cDeviceAddress = 0x3b;
    uint8_t micCount = 4;
    uint8_t ledCount = 16;
    const char *main_server_ip;

    // Server-specific settings
    bool use_terminal_input = false;
    bool use_home_assistant = false;
    std::string home_assistant_ip;
    std::string home_assistant_token;
    int home_assistant_port = 0;

#ifdef CLIENT_BUILD
    bool use_client = true;
#else
    bool use_client = false;
#endif

#ifdef SERVER_BUILD
    bool use_server = true;
#else
    bool use_server = false;
#endif

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
    static ConfigurationManager &getInstance();

    // Get the configuration for a specific client
    Configuration getConfiguration(const std::string &client_id);

    // Get the global configuration (for compatibility with existing code)
    Configuration getConfiguration();

    // Update the configuration for a specific client
    void updateConfiguration(const std::string &client_id, const Configuration &new_config);

    // Update the global configuration
    void updateConfiguration(const Configuration &new_config);

    // Get all configurations as a JSON object
    nlohmann::json getAllConfigurations();

    // Save the global configuration to a JSON file (for compatibility)
    void saveConfiguration(const std::string &filepath);

    // Load the global configuration from a JSON file (for compatibility)
    void loadConfiguration(const std::string &filepath);

    // Save all client configurations to a JSON file
    void saveConfigurations(const std::string &filepath);

    // Load all client configurations from a JSON file
    void loadConfigurations(const std::string &filepath);

    // Delete copy constructor and assignment operator
    ConfigurationManager(const ConfigurationManager &) = delete;
    ConfigurationManager &operator=(const ConfigurationManager &) = delete;

private:
    ConfigurationManager() {}
    std::unordered_map<std::string, Configuration> configurations;
    Configuration global_config; // For compatibility with single-client code
    std::mutex config_mutex;
};
#endif // CONFIGURATION_H
