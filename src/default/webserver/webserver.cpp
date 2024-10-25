/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    19-08-2024
 * @Date updated    08-10-2024 (By: Sebastiaan den Hertog)
 * @Description     Constructor, destructor, and methods for the webServer class.
 */

#include "webServer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
#include <memory>
#include "Configuration.h"

/**
 * @brief Setup the web server with the given parameters.
 */
using json = nlohmann::json;

/**
 * @brief Setup the web server with the given parameters.
 */
class GetConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        Configuration config = ConfigurationManager::getInstance().getConfiguration();
        json server_config;
        server_config["web_server_port"] = config.web_server_port;
        server_config["web_server_secure"] = config.web_server_secure;
        server_config["web_server_cert_path"] = config.web_server_cert_path;
        server_config["web_server_key_path"] = config.web_server_key_path;
        server_config["threads"] = config.threads;
        server_config["main_server_port"] = config.main_server_port;

        json client_config;
        client_config["client_id"] = config.client_id;
        client_config["client_server_ip"] = config.client_server_ip;
        client_config["use_airplay"] = config.use_airplay;
        client_config["use_bluetooth"] = config.use_bluetooth;
        client_config["use_client_server_connection"] = config.use_client_server_connection;
        client_config["use_home_assistant"] = config.use_home_assistant;
        client_config["home_assistant_ip"] = config.home_assistant_ip;
        client_config["home_assistant_port"] = config.home_assistant_port;
        client_config["home_assistant_token"] = config.home_assistant_token;
        client_config["web_client_port"] = config.web_client_port;

        // Add AirPlay-related settings
        client_config["airplay_server_name"] = config.airplay_server_name;
        client_config["airplay_audio_sync"] = config.airplay_audio_sync;
        client_config["airplay_video_sync"] = config.airplay_video_sync;
        client_config["airplay_audio_delay_alac"] = config.airplay_audio_delay_alac;
        client_config["airplay_audio_delay_aac"] = config.airplay_audio_delay_aac;
        client_config["airplay_relaunch_video"] = config.airplay_relaunch_video;
        client_config["airplay_reset_loop"] = config.airplay_reset_loop;
        client_config["airplay_open_connections"] = config.airplay_open_connections;
        client_config["airplay_videosink"] = config.airplay_videosink;
        client_config["airplay_use_video"] = config.airplay_use_video;
        client_config["airplay_compression_type"] = config.airplay_compression_type;
        client_config["airplay_audiosink"] = config.airplay_audiosink;
        client_config["airplay_audiodelay"] = config.airplay_audiodelay;
        client_config["airplay_use_audio"] = config.airplay_use_audio;
        client_config["airplay_new_window_closing_behavior"] = config.airplay_new_window_closing_behavior;
        client_config["airplay_close_window"] = config.airplay_close_window;
        client_config["airplay_video_parser"] = config.airplay_video_parser;
        client_config["airplay_video_decoder"] = config.airplay_video_decoder;
        client_config["airplay_video_converter"] = config.airplay_video_converter;
        client_config["airplay_show_client_FPS_data"] = config.airplay_show_client_FPS_data;
        client_config["airplay_max_ntp_timeouts"] = config.airplay_max_ntp_timeouts;
        client_config["airplay_dump_video"] = config.airplay_dump_video;
        client_config["airplay_dump_audio"] = config.airplay_dump_audio;
        client_config["airplay_audio_type"] = config.airplay_audio_type;
        client_config["airplay_previous_audio_type"] = config.airplay_previous_audio_type;
        client_config["airplay_fullscreen"] = config.airplay_fullscreen;
        client_config["airplay_do_append_hostname"] = config.airplay_do_append_hostname;
        client_config["airplay_use_random_hw_addr"] = config.airplay_use_random_hw_addr;
        client_config["airplay_restrict_clients"] = config.airplay_restrict_clients;
        client_config["airplay_setup_legacy_pairing"] = config.airplay_setup_legacy_pairing;
        client_config["airplay_require_password"] = config.airplay_require_password;
        client_config["airplay_pin"] = config.airplay_pin;
        client_config["airplay_db_low"] = config.airplay_db_low;
        client_config["airplay_db_high"] = config.airplay_db_high;
        client_config["airplay_taper_volume"] = config.airplay_taper_volume;
        client_config["airplay_h265_support"] = config.airplay_h265_support;
        client_config["airplay_n_renderers"] = config.airplay_n_renderers;

        json response_json;
        response_json["server_config"] = server_config;
        response_json["client_config"] = client_config;

        std::string response_body = response_json.dump(4);
        return std::make_shared<httpserver::string_response>(response_body, 200, "application/json");
    }
};

/**
 * @brief Resource to update the configuration settings.
 */
class UpdateConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_POST(const httpserver::http_request &req) override
    {
        try
        {
            json j = json::parse(req.get_content());
            Configuration current_config = ConfigurationManager::getInstance().getConfiguration();

            std::vector<std::pair<std::string, std::function<void(const json &)>>> server_mappings = {
                {"web_server_port", [&](const json &val)
                 { current_config.web_server_port = val.get<uint16_t>(); }},
                {"web_server_secure", [&](const json &val)
                 { current_config.web_server_secure = val.get<bool>(); }},
                {"web_server_cert_path", [&](const json &val)
                 { current_config.web_server_cert_path = val.get<std::string>(); }},
                {"web_server_key_path", [&](const json &val)
                 { current_config.web_server_key_path = val.get<std::string>(); }},
                {"threads", [&](const json &val)
                 { current_config.threads = val.get<int>(); }},
                {"main_server_port", [&](const json &val)
                 { current_config.main_server_port = val.get<uint16_t>(); }}};

            std::vector<std::pair<std::string, std::function<void(const json &)>>> client_mappings = {
                {"client_id", [&](const json &val)
                 { current_config.client_id = val.get<std::string>(); }},
                {"client_server_ip", [&](const json &val)
                 { current_config.client_server_ip = val.get<std::string>(); }},
                {"use_airplay", [&](const json &val)
                 { current_config.use_airplay = val.get<bool>(); }},
                {"use_bluetooth", [&](const json &val)
                 { current_config.use_bluetooth = val.get<bool>(); }},
                {"use_client_server_connection", [&](const json &val)
                 { current_config.use_client_server_connection = val.get<bool>(); }},
                {"use_home_assistant", [&](const json &val)
                 { current_config.use_home_assistant = val.get<bool>(); }},
                {"home_assistant_ip", [&](const json &val)
                 { current_config.home_assistant_ip = val.get<std::string>(); }},
                {"home_assistant_port", [&](const json &val)
                 { current_config.home_assistant_port = val.get<uint16_t>(); }},
                {"home_assistant_token", [&](const json &val)
                 { current_config.home_assistant_token = val.get<std::string>(); }},
                {"web_client_port", [&](const json &val)
                 { current_config.web_client_port = val.get<uint16_t>(); }},
                // Add mappings for AirPlay-related settings
                {"airplay_server_name", [&](const json &val)
                 { current_config.airplay_server_name = val.get<std::string>(); }},
                {"airplay_audio_sync", [&](const json &val)
                 { current_config.airplay_audio_sync = val.get<bool>(); }},
                {"airplay_video_sync", [&](const json &val)
                 { current_config.airplay_video_sync = val.get<bool>(); }},
                {"airplay_audio_delay_alac", [&](const json &val)
                 { current_config.airplay_audio_delay_alac = val.get<int64_t>(); }},
                {"airplay_audio_delay_aac", [&](const json &val)
                 { current_config.airplay_audio_delay_aac = val.get<int64_t>(); }},
                {"airplay_relaunch_video", [&](const json &val)
                 { current_config.airplay_relaunch_video = val.get<bool>(); }},
                {"airplay_reset_loop", [&](const json &val)
                 { current_config.airplay_reset_loop = val.get<bool>(); }},
                {"airplay_open_connections", [&](const json &val)
                 { current_config.airplay_open_connections = val.get<unsigned int>(); }},
                {"airplay_videosink", [&](const json &val)
                 { current_config.airplay_videosink = val.get<std::string>(); }},
                {"airplay_use_video", [&](const json &val)
                 { current_config.airplay_use_video = val.get<bool>(); }},
                {"airplay_compression_type", [&](const json &val)
                 { current_config.airplay_compression_type = val.get<unsigned char>(); }},
                {"airplay_audiosink", [&](const json &val)
                 { current_config.airplay_audiosink = val.get<std::string>(); }},
                {"airplay_audiodelay", [&](const json &val)
                 { current_config.airplay_audiodelay = val.get<int>(); }},
                {"airplay_use_audio", [&](const json &val)
                 { current_config.airplay_use_audio = val.get<bool>(); }},
                {"airplay_new_window_closing_behavior", [&](const json &val)
                 { current_config.airplay_new_window_closing_behavior = val.get<bool>(); }},
                {"airplay_close_window", [&](const json &val)
                 { current_config.airplay_close_window = val.get<bool>(); }},
                {"airplay_video_parser", [&](const json &val)
                 { current_config.airplay_video_parser = val.get<std::string>(); }},
                {"airplay_video_decoder", [&](const json &val)
                 { current_config.airplay_video_decoder = val.get<std::string>(); }},
                {"airplay_video_converter", [&](const json &val)
                 { current_config.airplay_video_converter = val.get<std::string>(); }},
                {"airplay_show_client_FPS_data", [&](const json &val)
                 { current_config.airplay_show_client_FPS_data = val.get<bool>(); }},
                {"airplay_max_ntp_timeouts", [&](const json &val)
                 { current_config.airplay_max_ntp_timeouts = val.get<unsigned int>(); }},
                {"airplay_dump_video", [&](const json &val)
                 { current_config.airplay_dump_video = val.get<bool>(); }},
                {"airplay_dump_audio", [&](const json &val)
                 { current_config.airplay_dump_audio = val.get<bool>(); }},
                {"airplay_audio_type", [&](const json &val)
                 { current_config.airplay_audio_type = val.get<unsigned char>(); }},
                {"airplay_previous_audio_type", [&](const json &val)
                 { current_config.airplay_previous_audio_type = val.get<unsigned char>(); }},
                {"airplay_fullscreen", [&](const json &val)
                 { current_config.airplay_fullscreen = val.get<bool>(); }},
                {"airplay_do_append_hostname", [&](const json &val)
                 { current_config.airplay_do_append_hostname = val.get<bool>(); }},
                {"airplay_use_random_hw_addr", [&](const json &val)
                 { current_config.airplay_use_random_hw_addr = val.get<bool>(); }},
                {"airplay_restrict_clients", [&](const json &val)
                 { current_config.airplay_restrict_clients = val.get<bool>(); }},
                {"airplay_setup_legacy_pairing", [&](const json &val)
                 { current_config.airplay_setup_legacy_pairing = val.get<bool>(); }},
                {"airplay_require_password", [&](const json &val)
                 { current_config.airplay_require_password = val.get<bool>(); }},
                {"airplay_pin", [&](const json &val)
                 { current_config.airplay_pin = val.get<unsigned short>(); }},
                {"airplay_db_low", [&](const json &val)
                 { current_config.airplay_db_low = val.get<double>(); }},
                {"airplay_db_high", [&](const json &val)
                 { current_config.airplay_db_high = val.get<double>(); }},
                {"airplay_taper_volume", [&](const json &val)
                 { current_config.airplay_taper_volume = val.get<bool>(); }},
                {"airplay_h265_support", [&](const json &val)
                 { current_config.airplay_h265_support = val.get<bool>(); }},
                {"airplay_n_renderers", [&](const json &val)
                 { current_config.airplay_n_renderers = val.get<int>(); }}};

            // Apply the mappings
            for (const auto &[key, updateFunc] : server_mappings)
            {
                if (j.contains(key))
                {
                    updateFunc(j[key]);
                }
            }

            for (const auto &[key, updateFunc] : client_mappings)
            {
                if (j.contains(key))
                {
                    updateFunc(j[key]);
                }
            }

            ConfigurationManager::getInstance().updateConfiguration(current_config);
            ConfigurationManager::getInstance().saveConfiguration("/config.json");

            json response_json = {
                {"status", "success"},
                {"message", "Configuration updated and saved"}};
            return std::make_shared<httpserver::string_response>(response_json.dump(), 200, "application/json");
        }
        catch (const std::exception &e)
        {
            json error_json = {
                {"status", "error"},
                {"message", e.what()}};
            return std::make_shared<httpserver::string_response>(error_json.dump(), 400, "application/json");
        }
    }
};

void custom_access_log(const std::string &url)
{
    std::cout << "ACCESSING: " << url << std::endl;
}

std::shared_ptr<httpserver::http_response> not_found_custom(const httpserver::http_request &)
{
    return std::shared_ptr<httpserver::string_response>(new httpserver::string_response("Not found custom", 404, "text/plain"));
}

std::shared_ptr<httpserver::http_response> not_allowed_custom(const httpserver::http_request &)
{
    return std::shared_ptr<httpserver::string_response>(new httpserver::string_response("Not allowed custom", 405, "text/plain"));
}

/**
 * @brief Resource to serve the configuration page.
 */
class ConfigPageResourceServer : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        std::ifstream html_file("webserver/private/server/pages/config_page.html");
        if (!html_file.is_open())
        {
            return std::make_shared<httpserver::string_response>("Config page not found.", 404, "text/plain");
        }

        std::stringstream buffer;
        buffer << html_file.rdbuf();
        return std::make_shared<httpserver::string_response>(buffer.str(), 200, "text/html");
    }
};

/**
 * @brief Resource to serve the configuration page.
 */
class ConfigPageResourceClient : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        std::ifstream html_file("webserver/private/client/pages/config_page.html");
        if (!html_file.is_open())
        {
            return std::make_shared<httpserver::string_response>("Config page not found.", 404, "text/plain");
        }

        std::stringstream buffer;
        buffer << html_file.rdbuf();
        return std::make_shared<httpserver::string_response>(buffer.str(), 200, "text/html");
    }
};

/**
 * @brief Resource to serve the home page.
 */
class HomePageServerResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        std::ifstream html_file("webserver/private/server/pages/home.html");
        if (!html_file.is_open())
        {
            return std::make_shared<httpserver::string_response>("Home page not found.", 404, "text/plain");
        }

        std::stringstream buffer;
        buffer << html_file.rdbuf();
        return std::make_shared<httpserver::string_response>(buffer.str(), 200, "text/html");
    }
};

/**
 * @brief Resource to serve the home page.
 */
class HomePageClientResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        std::ifstream html_file("webserver/private/client/pages/home.html");
        if (!html_file.is_open())
        {
            return std::make_shared<httpserver::string_response>("Home page not found.", 404, "text/plain");
        }

        std::stringstream buffer;
        buffer << html_file.rdbuf();
        return std::make_shared<httpserver::string_response>(buffer.str(), 200, "text/html");
    }
};

class ListClientsResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        try{
        // Get all client configurations as a JSON object
        json all_configs = ConfigurationManager::getInstance().getAllConfigurations();

        // Convert the JSON object to a formatted string
        std::string response_body = all_configs.dump(4);

        // Return the response with a 200 OK status and JSON content type
        return std::make_shared<httpserver::string_response>(response_body, 200, "application/json");
        }
        catch (const std::exception &e)
        {
            json error_json = {
                {"status", "error"},
                {"message", e.what()}};
            return std::make_shared<httpserver::string_response>(error_json.dump(), 400, "application/json");
        }
    }
};

class GetServerConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        try{
            
        json server_config = ConfigurationManager::getInstance().getConfiguration().to_json();
        return std::make_shared<httpserver::string_response>(server_config.dump(4), 200, "application/json");

        }
        catch (const std::exception &e)
        {
            json error_json = {
                {"status", "error"},
                {"message", e.what()}};
            return std::make_shared<httpserver::string_response>(error_json.dump(), 400, "application/json");
        }
    }
};

/**
 * @brief Resource to retrieve and update client-specific configurations.
 */
class GetClientConfigResourceOnlyServer : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        try{
        auto client_id = req.get_arg("client_id");

        // Check if there are no values for client_id
        if (client_id.get_flat_value().empty())
        {
            return std::make_shared<httpserver::string_response>("Client ID is required.", 400, "application/json");
        }

        json client_config = ConfigurationManager::getInstance().getConfiguration(client_id).to_json();

        return std::make_shared<httpserver::string_response>(client_config.dump(4), 200, "application/json");
        }
        catch (const std::exception &e)
        {
            json error_json = {
                {"status", "error"},
                {"message", e.what()}};
            return std::make_shared<httpserver::string_response>(error_json.dump(), 400, "application/json");
        }
    }
};

/**
 * @brief Resource to retrieve and update client-specific configurations.
 */
class GetClientConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        try{
        json client_config = ConfigurationManager::getInstance().getConfiguration().to_json();

        return std::make_shared<httpserver::string_response>(client_config.dump(4), 200, "application/json");
        }
        catch (const std::exception &e)
        {
            json error_json = {
                {"status", "error"},
                {"message", e.what()}};
            return std::make_shared<httpserver::string_response>(error_json.dump(), 400, "application/json");
        }
    }
};

// Endpoint to update the server configuration
class UpdateServerConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_POST(const httpserver::http_request &req) override
    {
        try
        {
            auto client_id = req.get_arg("client_id");

            // Check if there are no values for client_id
            if (client_id.get_flat_value().empty())
            {
                return std::make_shared<httpserver::string_response>("Client ID is required.", 400, "application/json");
            }

            // Parse the JSON body
            json j = json::parse(req.get_content());

            // Get and update the configuration for the specified client ID
            Configuration current_config = ConfigurationManager::getInstance().getConfiguration(client_id);
            current_config.from_json(j);
            ConfigurationManager::getInstance().updateConfiguration(client_id, current_config);
            ConfigurationManager::getInstance().saveConfigurations("/config.json");

            // Return a success response
            json response_json = {
                {"status", "success"},
                {"message", "Server configuration updated and saved"}};
            return std::make_shared<httpserver::string_response>(response_json.dump(), 200, "application/json");
        }
        catch (const std::exception &e)
        {
            json error_json = {
                {"status", "error"},
                {"message", e.what()}};
            return std::make_shared<httpserver::string_response>(error_json.dump(), 400, "application/json");
        }
    }
};

/**
 * @brief Resource to update a specific client's configuration.
 */
class UpdateClientConfigResourceOnlyServer : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_POST(const httpserver::http_request &req) override
    {
        try
        {
            auto client_id = req.get_arg("client_id");

            if (client_id.get_flat_value().empty())
            {
                return std::make_shared<httpserver::string_response>("Client ID is required.", 400, "application/json");
            }
            json j = json::parse(req.get_content());

            Configuration current_config = ConfigurationManager::getInstance().getConfiguration(client_id);

            current_config.from_json(j);
            ConfigurationManager::getInstance().updateConfiguration(client_id, current_config);
            ConfigurationManager::getInstance().saveConfigurations("/config.json");

            json response_json = {
                {"status", "success"},
                {"message", "Client configuration updated and saved"}};
            return std::make_shared<httpserver::string_response>(response_json.dump(), 200, "application/json");
        }
        catch (const std::exception &e)
        {
            json error_json = {
                {"status", "error"},
                {"message", e.what()}};
            return std::make_shared<httpserver::string_response>(error_json.dump(), 400, "application/json");
        }
    }
};

/**
 * @brief Resource to update a specific client's configuration.
 */
class UpdateClientConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_POST(const httpserver::http_request &req) override
    {
        try
        {

            json j = json::parse(req.get_content());

            Configuration current_config = ConfigurationManager::getInstance().getConfiguration();

            current_config.from_json(j);
            ConfigurationManager::getInstance().updateConfiguration(current_config);
            ConfigurationManager::getInstance().saveConfigurations("/config.json");

            json response_json = {
                {"status", "success"},
                {"message", "Client configuration updated and saved"}};
            return std::make_shared<httpserver::string_response>(response_json.dump(), 200, "application/json");
        }
        catch (const std::exception &e)
        {
            json error_json = {
                {"status", "error"},
                {"message", e.what()}};
            return std::make_shared<httpserver::string_response>(error_json.dump(), 400, "application/json");
        }
    }
};

/**
 * @brief Setup the web server with the given parameters.
 * @param secure Bool Whether to use SSL
 * @param cert Path to the SSL certificate
 * @param key Path to the SSL key
 * @param port Port number to listen on
 * @param threads Number of threads to use
 * @param use_server Bool Whether to use the server or client pages
 */

void setup_server(bool secure, const std::string &cert, const std::string &key, uint16_t port, int threads, bool use_server)
{
    try
    {
        httpserver::create_webserver ws_builder = httpserver::create_webserver(port).max_threads(threads).log_access(custom_access_log).not_found_resource(not_found_custom).method_not_allowed_resource(not_allowed_custom);
        if (key.empty() || cert.empty())
        {
            secure = false;
        }
        if (secure)
        {
            ws_builder.use_ssl().https_mem_key(key).https_mem_cert(cert);
            std::cout << "Using SSL with cert: " << cert << " and key: " << key << std::endl;
        }

        httpserver::webserver ws = httpserver::webserver(ws_builder);
        auto homePage = std::make_unique<HomePageServerResource>();
        ws.register_resource("/", homePage.get(), true);
        auto configPage = std::make_unique<ConfigPageResourceServer>();
        ws.register_resource("/config", configPage.get(), true);
#ifdef SERVER_BUILD

        auto getServerConfig = std::make_unique<GetServerConfigResource>();
        ws.register_resource("/api/server/config", getServerConfig.get(), true);
        auto updateServerConfig = std::make_unique<UpdateServerConfigResource>();
        ws.register_resource("/api/server/config/update", updateServerConfig.get(), true);
        auto listClients = std::make_unique<ListClientsResource>();
        ws.register_resource("/api/server/get_clients", listClients.get(), true);
#else

        auto getClientConfig = std::make_unique<GetClientConfigResource>();
        ws.register_resource("/api/client/config", getClientConfig.get(), true);
        auto updateClientConfig = std::make_unique<UpdateClientConfigResource>();
        ws.register_resource("/api/client/config/update", updateClientConfig.get(), true);
        auto getClientConfigOnlyServer = std::make_unique<GetClientConfigResourceOnlyServer>();
        ws.register_resource("/api/client/clientconfig", getClientConfigOnlyServer.get(), true);
        auto updateClientConfigOnlyServer = std::make_unique<UpdateClientConfigResourceOnlyServer>();
        ws.register_resource("/api/client/clientconfig/update", updateClientConfigOnlyServer.get(), true);

#endif
        ws.start(false);
        std::cout << "Web Server running on port: " << port << std::endl;

        while (ws.is_running())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "Web Server stopped" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server failed to start: " << e.what() << std::endl;
    }
}
