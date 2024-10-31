/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    13-07-2024
 * @Date updated    04-08-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the HomeAssistantAPI class
 */

#ifndef HOMEASSISTANTAPI_H
#define HOMEASSISTANTAPI_H

#include "NetworkManager.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>

class HomeAssistantAPI
{
public:
    HomeAssistantAPI(const std::string &host, int port, const std::string &token, NetworkManager *networkManager);
    ~HomeAssistantAPI();

    bool sendStateChange(const std::string &entityId, const std::string &newState);
    std::string getState(const std::string &entityId);
    std::vector<std::string> getEntityList();
    std::map<std::string, std::string> getEntityStates();
    bool callService(const std::string &domain, const std::string &service, const std::string &entityId);

private:
    std::string host;
    int port;
    std::string token;
    NetworkManager *networkManager;
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ip::tcp::socket socket;

    std::string sendRequest(const std::string &method, const std::string &target, const std::string &body = "");
    std::string receiveResponse();
};

#endif // HOMEASSISTANTAPI_H
