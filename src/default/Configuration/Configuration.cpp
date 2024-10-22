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
    j["use_server"] = use_server;
    j["use_client"] = use_client;
    j["main_server_port"] = main_server_port;
    j["client_server_ip"] = client_server_ip;
    j["web_client_port"] = web_client_port;
    j["use_airplay"] = use_airplay;
    j["use_client_server_connection"] = use_client_server_connection;
    j["use_bluetooth"] = use_bluetooth;
    j["use_server"] = use_server;
    j["home_assistant_ip"] = home_assistant_ip;
    j["home_assistant_port"] = home_assistant_port;
    j["home_assistant_token"] = home_assistant_token;
    // airplay
    j["airplay_server_name"] = airplay_server_name;
    j["airplay_audio_sync"] = airplay_audio_sync;
    j["airplay_video_sync"] = airplay_video_sync;
    j["airplay_audio_delay_alac"] = airplay_audio_delay_alac;
    j["airplay_audio_delay_aac"] = airplay_audio_delay_aac;
    j["airplay_relaunch_video"] = airplay_relaunch_video;
    j["airplay_reset_loop"] = airplay_reset_loop;
    j["airplay_open_connections"] = airplay_open_connections;
    j["airplay_videosink"] = airplay_videosink;
    j["airplay_use_video"] = airplay_use_video;
    j["airplay_compression_type"] = airplay_compression_type;
    j["airplay_audiosink"] = airplay_audiosink;
    j["airplay_audiodelay"] = airplay_audiodelay;
    j["airplay_use_audio"] = airplay_use_audio;
    j["airplay_new_window_closing_behavior"] = airplay_new_window_closing_behavior;
    j["airplay_close_window"] = airplay_close_window;
    j["airplay_video_parser"] = airplay_video_parser;
    j["airplay_video_decoder"] = airplay_video_decoder;
    j["airplay_video_converter"] = airplay_video_converter;
    j["airplay_show_client_FPS_data"] = airplay_show_client_FPS_data;
    j["airplay_max_ntp_timeouts"] = airplay_max_ntp_timeouts;
    j["airplay_dump_video"] = airplay_dump_video;
    j["airplay_dump_audio"] = airplay_dump_audio;
    j["airplay_audio_type"] = airplay_audio_type;
    j["airplay_previous_audio_type"] = airplay_previous_audio_type;
    j["airplay_fullscreen"] = airplay_fullscreen;
    j["airplay_do_append_hostname"] = airplay_do_append_hostname;
    j["airplay_use_random_hw_addr"] = airplay_use_random_hw_addr;
    j["airplay_restrict_clients"] = airplay_restrict_clients;
    j["airplay_setup_legacy_pairing"] = airplay_setup_legacy_pairing;
    j["airplay_require_password"] = airplay_require_password;
    j["airplay_pin"] = airplay_pin;
    j["airplay_db_low"] = airplay_db_low;
    j["airplay_db_high"] = airplay_db_high;
    j["airplay_taper_volume"] = airplay_taper_volume;
    j["airplay_h265_support"] = airplay_h265_support;
    j["airplay_n_renderers"] = airplay_n_renderers;
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
    if (j.contains("main_server_port"))
        main_server_port = j["main_server_port"];
    if (j.contains("main_server_ip") && !j["main_server_ip"].is_null())
    {
        std::string main_server_ip_str = j["main_server_ip"].get<std::string>();

        // If you need a C-style string (char*), use .c_str()
        main_server_ip = main_server_ip_str.c_str();
    }
    else
    {
        main_server_ip = nullptr;
    }
    if (j.contains("web_client_port"))
        web_client_port = j["web_client_port"];
    if (j.contains("use_airplay"))
        use_airplay = j["use_airplay"];
    if (j.contains("use_client_server_connection"))
        use_client_server_connection = j["use_client_server_connection"];
    if (j.contains("use_bluetooth"))
        use_bluetooth = j["use_bluetooth"];
    if (j.contains("home_assistant_ip"))
        home_assistant_ip = j["home_assistant_ip"];
    if (j.contains("home_assistant_port"))
        home_assistant_port = j["home_assistant_port"];
    if (j.contains("home_assistant_token"))
        home_assistant_token = j["home_assistant_token"];
    // airplay
    if (j.contains("airplay_server_name"))
        airplay_server_name = j["airplay_server_name"];
    if (j.contains("airplay_audio_sync"))
        airplay_audio_sync = j["airplay_audio_sync"];
    if (j.contains("airplay_video_sync"))
        airplay_video_sync = j["airplay_video_sync"];
    if (j.contains("airplay_audio_delay_alac"))
        airplay_audio_delay_alac = j["airplay_audio_delay_alac"];
    if (j.contains("airplay_audio_delay_aac"))
        airplay_audio_delay_aac = j["airplay_audio_delay_aac"];
    if (j.contains("airplay_relaunch_video"))
        airplay_relaunch_video = j["airplay_relaunch_video"];
    if (j.contains("airplay_reset_loop"))
        airplay_reset_loop = j["airplay_reset_loop"];
    if (j.contains("airplay_open_connections"))
        airplay_open_connections = j["airplay_open_connections"];
    if (j.contains("airplay_videosink"))
        airplay_videosink = j["airplay_videosink"];
    if (j.contains("airplay_use_video"))
        airplay_use_video = j["airplay_use_video"];
    if (j.contains("airplay_compression_type"))
        airplay_compression_type = j["airplay_compression_type"];
    if (j.contains("airplay_audiosink"))
        airplay_audiosink = j["airplay_audiosink"];
    if (j.contains("airplay_audiodelay"))
        airplay_audiodelay = j["airplay_audiodelay"];
    if (j.contains("airplay_use_audio"))
        airplay_use_audio = j["airplay_use_audio"];
    if (j.contains("airplay_new_window_closing_behavior"))
        airplay_new_window_closing_behavior = j["airplay_new_window_closing_behavior"];
    if (j.contains("airplay_close_window"))
        airplay_close_window = j["airplay_close_window"];
    if (j.contains("airplay_video_parser"))
        airplay_video_parser = j["airplay_video_parser"];
    if (j.contains("airplay_video_decoder"))
        airplay_video_decoder = j["airplay_video_decoder"];
    if (j.contains("airplay_video_converter"))
        airplay_video_converter = j["airplay_video_converter"];
    if (j.contains("airplay_show_client_FPS_data"))
        airplay_show_client_FPS_data = j["airplay_show_client_FPS_data"];
    if (j.contains("airplay_max_ntp_timeouts"))
        airplay_max_ntp_timeouts = j["airplay_max_ntp_timeouts"];
    if (j.contains("airplay_dump_video"))
        airplay_dump_video = j["airplay_dump_video"];
    if (j.contains("airplay_dump_audio"))
        airplay_dump_audio = j["airplay_dump_audio"];
    if (j.contains("airplay_audio_type"))
        airplay_audio_type = j["airplay_audio_type"];
    if (j.contains("airplay_previous_audio_type"))
        airplay_previous_audio_type = j["airplay_previous_audio_type"];
    if (j.contains("airplay_fullscreen"))
        airplay_fullscreen = j["airplay_fullscreen"];
    if (j.contains("airplay_do_append_hostname"))
        airplay_do_append_hostname = j["airplay_do_append_hostname"];
    if (j.contains("airplay_use_random_hw_addr"))
        airplay_use_random_hw_addr = j["airplay_use_random_hw_addr"];
    if (j.contains("airplay_restrict_clients"))
        airplay_restrict_clients = j["airplay_restrict_clients"];
    if (j.contains("airplay_setup_legacy_pairing"))
        airplay_setup_legacy_pairing = j["airplay_setup_legacy_pairing"];
    if (j.contains("airplay_require_password"))
        airplay_require_password = j["airplay_require_password"];
    if (j.contains("airplay_pin"))
        airplay_pin = j["airplay_pin"];
    if (j.contains("airplay_db_low"))
        airplay_db_low = j["airplay_db_low"];
    if (j.contains("airplay_db_high"))
        airplay_db_high = j["airplay_db_high"];
    if (j.contains("airplay_taper_volume"))
        airplay_taper_volume = j["airplay_taper_volume"];
    if (j.contains("airplay_h265_support"))
        airplay_h265_support = j["airplay_h265_support"];
    if (j.contains("airplay_n_renderers"))
        airplay_n_renderers = j["airplay_n_renderers"];
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