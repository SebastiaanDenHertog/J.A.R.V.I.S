#include "webServer.h"
#include <iostream>
#include <thread>
#include <chrono>

// Simple request handler
class ServiceResource : public httpserver::http_resource
{
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override
    {
        std::cout << "Received GET request on: " << req.get_path() << std::endl;
        return std::make_shared<httpserver::string_response>("GET response", 200);
    }

    std::shared_ptr<httpserver::http_response> render_POST(const httpserver::http_request &req) override
    {
        std::cout << "Received POST request on: " << req.get_path() << std::endl;
        return std::make_shared<httpserver::string_response>("POST response", 200);
    }

    std::shared_ptr<httpserver::http_response> render_PUT(const httpserver::http_request &req) override
    {
        std::cout << "Received PUT request on: " << req.get_path() << std::endl;
        return std::make_shared<httpserver::string_response>("PUT response", 200);
    }

    std::shared_ptr<httpserver::http_response> render_DELETE(const httpserver::http_request &req) override
    {
        std::cout << "Received DELETE request on: " << req.get_path() << std::endl;
        return std::make_shared<httpserver::string_response>("DELETE response", 200);
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
        ServiceResource *res = new ServiceResource();
        ws.register_resource("/service", res, true);

        ws.start(false); // No ambiguity here, fully qualified call
        std::cout << "Server running on port: " << port << std::endl;

        while (ws.is_running())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "Server stopped" << std::endl;
        ws.stop();
        delete res;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server failed to start: " << e.what() << std::endl;
    }
}

class ConfigurationResource : public httpserver::http_resource {
public:
    std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request &req) override {
        auto& config = ConfigurationManager::getInstance();
        std::ostringstream oss;
        oss << "{"
            << "\"useWebServer\": " << (config.getUseWebServer() ? "true" : "false") << ","
            << "\"webServerPort\": " << config.getWebServerPort() << ","
            << "\"mainServerIP\": \"" << config.getMainServerIP() << "\","
            << "\"mainServerPort\": " << config.getMainServerPort() << ","
            << "\"useBluetooth\": " << (config.getUseBluetooth() ? "true" : "false") << ","
            << "\"useAirPlay\": " << (config.getUseAirPlay() ? "true" : "false")
            << "}";
        return std::make_shared<httpserver::string_response>(oss.str(), 200, "application/json");
    }

    std::shared_ptr<httpserver::http_response> render_POST(const httpserver::http_request &req) override {
        auto& config = ConfigurationManager::getInstance();
        const auto& body = req.get_content();

        // Parse JSON body (use a JSON library or manual parsing)
        // Here, we assume simple parsing; in a real scenario, use a JSON library like nlohmann::json.

        if (body.find("useWebServer") != std::string::npos) {
            config.setUseWebServer(body.find("true") != std::string::npos);
        }

        if (body.find("webServerPort") != std::string::npos) {
            unsigned short port = /* extract port from body */;
            config.setWebServerPort(port);
        }

        if (body.find("mainServerIP") != std::string::npos) {
            std::string ip = /* extract IP from body */;
            config.setMainServerIP(ip);
        }

        if (body.find("mainServerPort") != std::string::npos) {
            int port = /* extract port from body */;
            config.setMainServerPort(port);
        }

        // Other configuration updates...

        return std::make_shared<httpserver::string_response>("Configuration updated", 200);
    }
};

// In setup_server function
ConfigurationResource* configRes = new ConfigurationResource();
ws.register_resource("/config", configRes, true);
