#ifndef HOMEASSISTANTAPI_H
#define HOMEASSISTANTAPI_H

#include "NetworkManager.h"
#include <string>
#include <vector>
#include <map>

class HomeAssistantAPI
{
public:
    HomeAssistantAPI(const std::string &host, int port, NetworkManager *networkManager);
    ~HomeAssistantAPI();

    bool sendStateChange(const std::string &entityId, const std::string &newState);
    std::string getState(const std::string &entityId);
    std::vector<std::string> getEntityList();
    std::map<std::string, std::string> getEntityStates();
    bool callService(const std::string &domain, const std::string &service, const std::string &entityId);

private:
    std::string host;
    int port;
    NetworkManager *networkManager;
    int serverSd;

    std::string sendRequest(const std::string &request);
};

#endif // HOMEASSISTANTAPI_H
