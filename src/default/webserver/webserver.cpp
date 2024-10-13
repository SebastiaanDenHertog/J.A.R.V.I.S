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
        json j;
        j["web_server_port"] = config.web_server_port;
        j["web_server_secure"] = config.web_server_secure;
        j["web_server_cert_path"] = config.web_server_cert_path;
        j["web_server_key_path"] = config.web_server_key_path;
        j["threads"] = config.threads;
        j["main_server_port"] = config.main_server_port;
        j["client_server_ip"] = config.client_server_ip;
        j["use_airplay"] = config.use_airplay;
        j["use_bluetooth"] = config.use_bluetooth;
        j["use_client_server_connection"] = config.use_client_server_connection;
        j["use_home_assistant"] = config.use_home_assistant;
        j["home_assistant_ip"] = config.home_assistant_ip;
        j["home_assistant_port"] = config.home_assistant_port;
        j["home_assistant_token"] = config.home_assistant_token;

        std::string response_body = j.dump(4);
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
            // Parse JSON body
            json j = json::parse(req.get_content());
            Configuration current_config = ConfigurationManager::getInstance().getConfiguration();
            // Update configuration fields using a map-like approach for brevity
            std::vector<std::pair<std::string, std::function<void(const json &)>>> mappings = {
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
                {"main_server_port", [&](const json &val)
                 { current_config.main_server_port = val; }},
                {"client_server_ip", [&](const json &val)
                 { current_config.client_server_ip = val; }},
                {"use_airplay", [&](const json &val)
                 { current_config.use_airplay = val; }},
                {"use_bluetooth", [&](const json &val)
                 { current_config.use_bluetooth = val; }},
                {"use_client_server_connection", [&](const json &val)
                 { current_config.use_client_server_connection = val; }},
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

class ListClientsResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        // Get all client configurations as a JSON object
        json all_configs = ConfigurationManager::getInstance().getAllConfigurations();

        // Convert the JSON object to a formatted string
        std::string response_body = all_configs.dump(4);

        // Return the response with a 200 OK status and JSON content type
        return std::make_shared<httpserver::string_response>(response_body, 200, "application/json");
    }
};

class GetServerConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        json server_config = ConfigurationManager::getInstance().getConfiguration().to_json();
        return std::make_shared<httpserver::string_response>(server_config.dump(4), 200, "application/json");
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
            json j = json::parse(req.get_content());
            Configuration current_config = ConfigurationManager::getInstance().getConfiguration();
            current_config.from_json(j);
            ConfigurationManager::getInstance().updateConfiguration(current_config);
            ConfigurationManager::getInstance().saveConfiguration("config.json");

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

class UpdateClientConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_POST(const httpserver::http_request &req) override
    {
        try
        {
            for (auto client_id_test = req.get_args(); client_id_test.size() > 0;)
            {
                for (auto client_id = client_id_test.at("client_id").values; client_id.size() > 0; client_id.pop_back())
                {
                    std::cout << client_id.back() << std::endl;
                }
            }

            std::string client_id = "first";
            if (client_id.empty())
            {
                return std::make_shared<httpserver::string_response>("Client ID is required.", 400, "application/json");
            }

            json j = json::parse(req.get_content());
            Configuration current_config = ConfigurationManager::getInstance().getConfiguration(client_id);
            current_config.from_json(j);
            ConfigurationManager::getInstance().updateConfiguration(client_id, current_config);
            ConfigurationManager::getInstance().saveConfigurations("config.json");

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

        auto homePage = std::make_unique<HomePageResource>();
        ws.register_resource("/", homePage.get(), true);
        if (use_server)
        {
            auto configPage = std::make_unique<ConfigPageResourceServer>();
            ws.register_resource("/config", configPage.get(), true);
        }
        else
        {
            auto configPage = std::make_unique<ConfigPageResourceClient>();
            ws.register_resource("/config", configPage.get(), true);
        }
        auto getServerConfig = std::make_unique<GetServerConfigResource>();
        ws.register_resource("/api/server/config", getServerConfig.get(), true);

        auto updateServerConfig = std::make_unique<UpdateServerConfigResource>();
        ws.register_resource("/api/server/config/update", updateServerConfig.get(), true);

        auto listClients = std::make_unique<ListClientsResource>();
        ws.register_resource("/api/clients", listClients.get(), true);

        auto updateClientConfig = std::make_unique<UpdateClientConfigResource>();
        ws.register_resource("/api/client/config/update", updateClientConfig.get(), true);

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
