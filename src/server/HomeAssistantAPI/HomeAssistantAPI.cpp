/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    13-07-2024
 * @Date updated    24-08-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the HomeAssistantAPI class
 **/

#include "HomeAssistantAPI.h"
#include <iostream>
#include <boost/asio/connect.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace http = boost::beast::http;

HomeAssistantAPI::HomeAssistantAPI(const std::string &host, int port, const std::string &token, NetworkManager *networkManager)
    : host(host), port(port), token(token), networkManager(networkManager), resolver(ioc), socket(ioc)
{
    try
    {
        auto const results = resolver.resolve(host, std::to_string(port));
        boost::asio::connect(socket, results.begin(), results.end());
        std::cout << "Connected to Home Assistant" << std::endl;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to connect to Home Assistant" + std::string(e.what()));
    }
}

HomeAssistantAPI::~HomeAssistantAPI()
{
    boost::system::error_code ec;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket.close(ec);
}

std::string HomeAssistantAPI::sendRequest(const std::string &method, const std::string &target, const std::string &body)
{
    try
    {
        http::request<http::string_body> req;
        req.method(http::string_to_verb(method));
        req.target(target);
        req.set(http::field::host, host);
        req.set(http::field::authorization, "Bearer " + token);
        req.set(http::field::content_type, "application/json");
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.body() = body;
        req.prepare_payload();

        http::write(socket, req);

        return receiveResponse();
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to send request to Home Assistant" + std::string(e.what()));
    }
}

std::string HomeAssistantAPI::receiveResponse()
{
    try
    {
        boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(socket, buffer, res);
        return boost::beast::buffers_to_string(res.body().data());
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to receive response from Home Assistant" + std::string(e.what()));
    }
}

bool HomeAssistantAPI::sendStateChange(const std::string &entityId, const std::string &newState)
{
    nlohmann::json body;
    body["state"] = newState;

    std::string target = "/api/states/" + entityId;
    std::string response = sendRequest("POST", target, body.dump());

    return response.find("\"state\": \"" + newState + "\"") != std::string::npos;
}

std::string HomeAssistantAPI::getState(const std::string &entityId)
{
    std::string target = "/api/states/" + entityId;
    return sendRequest("GET", target);
}

std::vector<std::string> HomeAssistantAPI::getEntityList()
{
    std::string target = "/api/states";
    std::string response = sendRequest("GET", target);

    std::vector<std::string> entities;
    auto json_response = nlohmann::json::parse(response);
    for (auto &item : json_response)
    {
        entities.push_back(item["entity_id"]);
    }
    return entities;
}

std::map<std::string, std::string> HomeAssistantAPI::getEntityStates()
{
    std::string target = "/api/states";
    std::string response = sendRequest("GET", target);

    std::map<std::string, std::string> entityStates;
    auto json_response = nlohmann::json::parse(response);
    for (auto &item : json_response)
    {
        entityStates[item["entity_id"]] = item["state"];
    }
    return entityStates;
}

bool HomeAssistantAPI::callService(const std::string &domain, const std::string &service, const std::string &entityId)
{
    nlohmann::json body;
    body["entity_id"] = entityId;

    std::string target = "/api/services/" + domain + "/" + service;
    std::string response = sendRequest("POST", target, body.dump());

    return response.find("\"success\": true") != std::string::npos;
}
