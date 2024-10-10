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
    if (j.contains("client_server_ip"))
        client_server_ip = j["client_server_ip"];
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
 * @brief Update the configuration with a new configuration.
 * @param filepath The path to the JSON file to save the configuration to.
 */

void ConfigurationManager::saveConfiguration(const std::string &filepath)
{
    std::lock_guard<std::mutex> lock(config_mutex);
    nlohmann::json j = config.to_json();
    std::ofstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open configuration file for writing: " << filepath << std::endl;
        return;
    }
    file << j.dump(4);
    file.close();
    std::cout << "Configuration saved to " << filepath << std::endl;
}

/**
 * @brief Load the configuration from a JSON file.
 * @param filepath The path to the JSON file to load the configuration from.
 */

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
        config.from_json(j);
        std::cout << "Configuration loaded from " << filepath << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing configuration file: " << e.what() << ". Using default settings." << std::endl;
    }

    file.close();
}
