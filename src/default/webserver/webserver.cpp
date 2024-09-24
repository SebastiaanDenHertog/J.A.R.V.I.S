#include "webServer.h"
#include <httpserver.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace httpserver;

// Simple request handler
class ServiceResource : public http_resource
{
public:
    std::shared_ptr<http_response> render_GET(const http_request &req) override
    {
        std::cout << "Received GET request on: " << req.get_path() << std::endl;
        return std::make_shared<string_response>("GET response", 200);
    }

    std::shared_ptr<http_response> render_POST(const http_request &req) override
    {
        std::cout << "Received POST request on: " << req.get_path() << std::endl;
        return std::make_shared<string_response>("POST response", 200);
    }

    std::shared_ptr<http_response> render_PUT(const http_request &req) override
    {
        std::cout << "Received PUT request on: " << req.get_path() << std::endl;
        return std::make_shared<string_response>("PUT response", 200);
    }

    std::shared_ptr<http_response> render_DELETE(const http_request &req) override
    {
        std::cout << "Received DELETE request on: " << req.get_path() << std::endl;
        return std::make_shared<string_response>("DELETE response", 200);
    }
};

void setup_server(bool secure, const std::string &cert, const std::string &key, uint16_t port, int threads)
{
    try
    {
        create_webserver ws_builder = create_webserver(port).max_threads(threads);
        if (key.empty() || cert.empty())
        {
            secure = false;
        }
        if (secure)
        {
            ws_builder.use_ssl().https_mem_key(key).https_mem_cert(cert);
            std::cout << "Using SSL with cert: " << cert << " and key: " << key << std::endl;
        }

        webserver ws = webserver(ws_builder);
        ServiceResource *res = new ServiceResource();
        ws.register_resource("/service", res, true);

        ws.start(false);
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
