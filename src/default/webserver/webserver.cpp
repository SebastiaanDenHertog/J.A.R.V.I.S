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
        client_config["client_server_ip"] = config.client_server_ip;
        client_config["use_airplay"] = config.use_airplay;
        client_config["use_bluetooth"] = config.use_bluetooth;
        client_config["use_client_server_connection"] = config.use_client_server_connection;
        client_config["use_home_assistant"] = config.use_home_assistant;
        client_config["home_assistant_ip"] = config.home_assistant_ip;
        client_config["home_assistant_port"] = config.home_assistant_port;
        client_config["home_assistant_token"] = config.home_assistant_token;
        client_config["web_client_port"] = config.web_client_port;

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
                 { current_config.web_client_port = val.get<uint16_t>(); }}};

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

/**
 * @brief Resource to retrieve and update client-specific configurations.
 */
class GetClientConfigResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        auto client_id = req.get_arg("client_id");

        // Check if there are no values for client_id
        if (client_id.get_flat_value().empty())
        {
            return std::make_shared<httpserver::string_response>("Client ID is required.", 400, "application/json");
        }

        json client_config = ConfigurationManager::getInstance().getConfiguration(client_id).to_json();

        return std::make_shared<httpserver::string_response>(client_config.dump(4), 200, "application/json");
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
            ConfigurationManager::getInstance().saveConfigurations("config.json");

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
class UpdateClientConfigResource : public httpserver::http_resource
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

#ifdef SERVER_BUILD
        auto homePage = std::make_unique<HomePageServerResource>();
        ws.register_resource("/", homePage.get(), true);
        auto configPage = std::make_unique<ConfigPageResourceServer>();
        ws.register_resource("/config", configPage.get(), true);

        auto getServerConfig = std::make_unique<GetServerConfigResource>();
        ws.register_resource("/api/server/config", getServerConfig.get(), true);
        auto updateServerConfig = std::make_unique<UpdateServerConfigResource>();
        ws.register_resource("/api/server/config/update", updateServerConfig.get(), true);
        auto listClients = std::make_unique<ListClientsResource>();
        ws.register_resource("/api/server/get_clients", listClients.get(), true);
#else
        auto homePage = std::make_unique<HomePageClientResource>();
        ws.register_resource("/", homePage.get(), true);
        auto configPage = std::make_unique<ConfigPageResourceClient>();
        ws.register_resource("/config", configPage.get(), true);

        auto getClientConfig = std::make_unique<GetClientConfigResource>();
        ws.register_resource("/api/client/config", getClientConfig.get(), true);
        auto updateClientConfig = std::make_unique<UpdateClientConfigResource>();
        ws.register_resource("/api/client/config/update", updateClientConfig.get(), true);
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
