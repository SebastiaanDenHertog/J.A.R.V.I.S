/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    19-08-2024
 * @Date updated    08-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the webServer class
 **/

#include "webServer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
#include "Configuration.h"

// For simplicity, using nlohmann::json
using json = nlohmann::json;

// Endpoint to get current configuration
class GetConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        Configuration config = ConfigurationManager::getInstance().getConfiguration();
        json j;
        j["use_web_server"] = config.use_web_server;
        j["web_server_port"] = config.web_server_port;
        j["web_server_secure"] = config.web_server_secure;
        j["web_server_cert_path"] = config.web_server_cert_path;
        j["web_server_key_path"] = config.web_server_key_path;
        j["threads"] = config.threads;
        j["use_client"] = config.use_client;
        j["main_server_port"] = config.main_server_port;
        j["client_server_ip"] = config.client_server_ip;
        j["use_airplay"] = config.use_airplay;
        j["use_bluetooth"] = config.use_bluetooth;
        j["use_server"] = config.use_server;
        // Add other fields as needed

        std::string response_body = j.dump(4);
        return std::make_shared<httpserver::string_response>(response_body, 200, "application/json");
    }
};

// Endpoint to update configuration
class UpdateConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_POST(const httpserver::http_request &req) override
    {
        try
        {
            // Parse JSON body
            json j = json::parse(req.get_content());

            Configuration current_config = ConfigurationManager::getInstance().getConfiguration();

            // Update fields if they exist in the JSON
            if (j.contains("use_web_server"))
            {
                current_config.use_web_server = j["use_web_server"];
            }
            if (j.contains("web_server_port"))
            {
                current_config.web_server_port = j["web_server_port"];
            }
            if (j.contains("web_server_secure"))
            {
                current_config.web_server_secure = j["web_server_secure"];
            }
            if (j.contains("web_server_cert_path"))
            {
                current_config.web_server_cert_path = j["web_server_cert_path"];
            }
            if (j.contains("web_server_key_path"))
            {
                current_config.web_server_key_path = j["web_server_key_path"];
            }
            if (j.contains("threads"))
            {
                current_config.threads = j["threads"];
            }
            if (j.contains("use_client"))
            {
                current_config.use_client = j["use_client"];
            }
            if (j.contains("client_port"))
            {
                current_config.main_server_port = j["main_server_port"];
            }
            if (j.contains("client_server_ip"))
            {
                current_config.client_server_ip = j["client_server_ip"];
            }
            if (j.contains("use_airplay"))
            {
                current_config.use_airplay = j["use_airplay"];
            }
            if (j.contains("use_bluetooth"))
            {
                current_config.use_bluetooth = j["use_bluetooth"];
            }
            if (j.contains("use_server"))
            {
                current_config.use_server = j["use_server"];
            }
            // Add other fields as needed

            // Update the configuration
            ConfigurationManager::getInstance().updateConfiguration(current_config);

            // Save the updated configuration to file
            ConfigurationManager::getInstance().saveConfiguration("config.json");

            // Respond with the updated configuration
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

// Endpoint to serve a simple HTML configuration page
class ConfigPageResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        std::ifstream html_file("config_page.html");
        if (!html_file.is_open())
        {
            return std::make_shared<httpserver::string_response>("Config page not found.", 404, "text/plain");
        }

        std::stringstream buffer;
        buffer << html_file.rdbuf();
        return std::make_shared<httpserver::string_response>(buffer.str(), 200, "text/html");
    }
};

void setup_server(bool secure, const std::string &cert, const std::string &key, uint16_t port, int threads)
{
    try
    {
        httpserver::create_webserver ws_builder = httpserver::create_webserver(port).max_threads(threads);
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

        // Register API resources
        GetConfigResource *getConfig = new GetConfigResource();
        ws.register_resource("/api/config", getConfig, true);

        UpdateConfigResource *updateConfig = new UpdateConfigResource();
        ws.register_resource("/api/config/update", updateConfig, true);

        // Register HTML config page
        ConfigPageResource *configPage = new ConfigPageResource();
        ws.register_resource("/config", configPage, true);

        ws.start(false); // No ambiguity here, fully qualified call
        std::cout << "Web Server running on port: " << port << std::endl;

        while (ws.is_running())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "Web Server stopped" << std::endl;
        ws.stop();
        delete getConfig;
        delete updateConfig;
        delete configPage;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server failed to start: " << e.what() << std::endl;
    }
}