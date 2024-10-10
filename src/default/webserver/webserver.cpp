/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    19-08-2024
 * @Date updated    08-10-2024 (By: Sebastiaan den Hertog)
 * @Description     Constructor, destructor, and methods for the webServer class.
 **/

#include "webServer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
#include <memory>
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
        j["use_home_assistant"] = config.use_home_assistant;
        j["home_assistant_ip"] = config.home_assistant_ip;
        j["home_assistant_port"] = config.home_assistant_port;
        j["home_assistant_token"] = config.home_assistant_token;

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
            // Update configuration fields using a map-like approach for brevity
            std::vector<std::pair<std::string, std::function<void(const json &)>>> mappings = {
                {"use_web_server", [&](const json &val)
                 { current_config.use_web_server = val; }},
                {"web_server_port", [&](const json &val)
                 { current_config.web_server_port = val; }},
                {"web_server_secure", [&](const json &val)
                 { current_config.web_server_secure = val; }},
                {"web_server_cert_path", [&](const json &val)
                 { current_config.web_server_cert_path = val; }},
                {"web_server_key_path", [&](const json &val)
                 { current_config.web_server_key_path = val; }},
                {"threads", [&](const json &val)
                 { current_config.threads = val; }},
                {"use_client", [&](const json &val)
                 { current_config.use_client = val; }},
                {"main_server_port", [&](const json &val)
                 { current_config.main_server_port = val; }},
                {"client_server_ip", [&](const json &val)
                 { current_config.client_server_ip = val; }},
                {"use_airplay", [&](const json &val)
                 { current_config.use_airplay = val; }},
                {"use_bluetooth", [&](const json &val)
                 { current_config.use_bluetooth = val; }},
                {"use_server", [&](const json &val)
                 { current_config.use_server = val; }},
                {"use_home_assistant", [&](const json &val)
                 { current_config.use_home_assistant = val; }},
                {"home_assistant_ip", [&](const json &val)
                 { current_config.home_assistant_ip = val; }},
                {"home_assistant_port", [&](const json &val)
                 { current_config.home_assistant_port = val; }},
                {"home_assistant_token", [&](const json &val)
                 { current_config.home_assistant_token = val; }}};

            for (const auto &[key, updateFunc] : mappings)
            {
                if (j.contains(key))
                {
                    updateFunc(j[key]);
                }
            }

            ConfigurationManager::getInstance().updateConfiguration(current_config);
            ConfigurationManager::getInstance().saveConfiguration("config.json");

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
        std::ifstream html_file("webserver/private/pages/config_page.html");
        if (!html_file.is_open())
        {
            return std::make_shared<httpserver::string_response>("Config page not found.", 404, "text/plain");
        }

        std::stringstream buffer;
        buffer << html_file.rdbuf();
        return std::make_shared<httpserver::string_response>(buffer.str(), 200, "text/html");
    }
};

class HomePageResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        std::ifstream html_file("webserver/private/pages/home.html");
        if (!html_file.is_open())
        {
            return std::make_shared<httpserver::string_response>("Home page not found.", 404, "text/plain");
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

        // Register resources using smart pointers for automatic cleanup
        auto homePage = std::make_unique<HomePageResource>();
        ws.register_resource("/", homePage.get(), true);

        auto configPage = std::make_unique<ConfigPageResource>();
        ws.register_resource("/config", configPage.get(), true);

        auto getConfig = std::make_unique<GetConfigResource>();
        ws.register_resource("/api/config", getConfig.get(), true);

        auto updateConfig = std::make_unique<UpdateConfigResource>();
        ws.register_resource("/api/config/update", updateConfig.get(), true);

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
