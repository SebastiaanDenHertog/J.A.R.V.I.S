#include "Configuration.h"
#include <fstream>
#include <iostream>

/**
 * @brief Save the configuration to a JSON file.
 * @return The JSON object representing the configuration.
 */

nlohmann::json Configuration::to_json() const
{
    nlohmann::json j;
    j["use_web_server"] = use_web_server;
    j["web_server_port"] = web_server_port;
    j["web_server_secure"] = web_server_secure;
    j["web_server_cert_path"] = web_server_cert_path;
    j["web_server_key_path"] = web_server_key_path;
    j["threads"] = threads;
    j["use_client"] = use_client;
    j["main_server_port"] = main_server_port;
    j["client_server_ip"] = client_server_ip;
    j["use_airplay"] = use_airplay;
    j["use_bluetooth"] = use_bluetooth;
    j["use_server"] = use_server;
    j["home_assistant_ip"] = home_assistant_ip;
    j["home_assistant_port"] = home_assistant_port;
    j["home_assistant_token"] = home_assistant_token;
    return j;
}

/**
 * @brief Load the configuration from a JSON object.
 * @param j The JSON object to load the configuration from.
 */

void Configuration::from_json(const nlohmann::json &j)
{
    if (j.contains("use_web_server"))
        use_web_server = j["use_web_server"];
    if (j.contains("web_server_port"))
        web_server_port = j["web_server_port"];
    if (j.contains("web_server_secure"))
        web_server_secure = j["web_server_secure"];
    if (j.contains("web_server_cert_path"))
        web_server_cert_path = j["web_server_cert_path"];
    if (j.contains("web_server_key_path"))
        web_server_key_path = j["web_server_key_path"];
    if (j.contains("threads"))
        threads = j["threads"];
    if (j.contains("use_client"))
        use_client = j["use_client"];
    if (j.contains("main_server_port"))
        main_server_port = j["main_server_port"];
    if (j.contains("main_server_ip") && !j["main_server_ip"].is_null()) {
        main_server_ip = j["main_server_ip"];
    } else {
        main_server_ip = nullptr;
    }
    if (j.contains("use_airplay"))
        use_airplay = j["use_airplay"];
    if (j.contains("use_bluetooth"))
        use_bluetooth = j["use_bluetooth"];
    if (j.contains("use_server"))
        use_server = j["use_server"];
    if (j.contains("home_assistant_ip"))
        home_assistant_ip = j["home_assistant_ip"];
    if (j.contains("home_assistant_port"))
        home_assistant_port = j["home_assistant_port"];
    if (j.contains("home_assistant_token"))
        home_assistant_token = j["home_assistant_token"];
}
/**
 * 
 */

ConfigurationManager &ConfigurationManager::getInstance()
{
    static ConfigurationManager instance;
    return instance;
}

/**
 * 
 */
Configuration ConfigurationManager::getConfiguration(const std::string &client_id)
{
    std::lock_guard<std::mutex> lock(config_mutex);
    return configurations[client_id];
}

/**
 * 
 */

Configuration ConfigurationManager::getConfiguration()
{
    std::lock_guard<std::mutex> lock(config_mutex);
    return global_config;
}

/**
 * 
 */
void ConfigurationManager::updateConfiguration(const std::string &client_id, const Configuration &new_config)
{
    std::lock_guard<std::mutex> lock(config_mutex);
    configurations[client_id] = new_config;
}

/**
 * 
 */
void ConfigurationManager::updateConfiguration(const Configuration &new_config)
{
    std::lock_guard<std::mutex> lock(config_mutex);
    global_config = new_config;
}

/**
 * 
 */
nlohmann::json ConfigurationManager::getAllConfigurations()
{
    std::lock_guard<std::mutex> lock(config_mutex);
    nlohmann::json j;
    for (const auto &pair : configurations)
    {
        j[pair.first] = pair.second.to_json();
    }
    return j;
}

/**
 * 
 */
// Save all client configurations to a JSON file
void ConfigurationManager::saveConfigurations(const std::string &filepath)
{
    std::lock_guard<std::mutex> lock(config_mutex);
    nlohmann::json j = getAllConfigurations();
    std::ofstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open configuration file for writing: " << filepath << std::endl;
        return;
    }
    file << j.dump(4);
    std::cout << "Configurations saved to " << filepath << std::endl;
}

// Save the global configuration to a JSON file
void ConfigurationManager::saveConfiguration(const std::string &filepath)
{
    std::lock_guard<std::mutex> lock(config_mutex);
    nlohmann::json j = global_config.to_json();
    std::ofstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open configuration file for writing: " << filepath << std::endl;
        return;
    }
    file << j.dump(4);
    std::cout << "Configuration saved to " << filepath << std::endl;
}
/**
 * 
 */
// Load all client configurations from a JSON file
void ConfigurationManager::loadConfigurations(const std::string &filepath)
{
    std::lock_guard<std::mutex> lock(config_mutex);
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Configuration file not found: " << filepath << ". Using default settings." << std::endl;
        return;
    }

    nlohmann::json j;
    try
    {
        file >> j;
        for (const auto &item : j.items())
        {
            const std::string &client_id = item.key();
            Configuration config;
            config.from_json(item.value());
            configurations[client_id] = config;
        }
        std::cout << "Configurations loaded from " << filepath << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing configuration file: " << e.what() << ". Using default settings." << std::endl;
    }
}
/**
 * 
 */
// Load the global configuration from a JSON file
void ConfigurationManager::loadConfiguration(const std::string &filepath)
{
    std::lock_guard<std::mutex> lock(config_mutex);
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Configuration file not found: " << filepath << ". Using default settings." << std::endl;
        return;
    }

    nlohmann::json j;
    try
    {
        file >> j;
        global_config.from_json(j);
        std::cout << "Configuration loaded from " << filepath << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing configuration file: " << e.what() << ". Using default settings." << std::endl;
    }
}