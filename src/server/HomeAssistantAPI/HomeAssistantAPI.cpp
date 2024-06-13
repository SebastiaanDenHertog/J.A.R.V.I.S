#include "HomeAssistantAPI.h"
#include <sstream>
#include <cstring> // Include cstring for memset

HomeAssistantAPI::HomeAssistantAPI(const std::string &host, int port, NetworkManager *networkManager)
    : host(host), port(port), networkManager(networkManager)
{
    serverSd = networkManager->getServerSocket(); // Get server socket descriptor from NetworkManager
}

HomeAssistantAPI::~HomeAssistantAPI()
{
}

std::string HomeAssistantAPI::sendRequest(const std::string &request)
{
    networkManager->send(serverSd, request.c_str(), request.length(), 0);

    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = networkManager->recv(serverSd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived < 0)
    {
        perror("Failed to read data from server");
        return "";
    }

    return std::string(buffer, bytesReceived);
}

bool HomeAssistantAPI::sendStateChange(const std::string &entityId, const std::string &newState)
{
    std::ostringstream request;
    request << "POST /api/states/" << entityId << " HTTP/1.1\r\n";
    request << "Host: " << host << ":" << port << "\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Content-Length: " << (newState.length() + 20) << "\r\n\r\n";
    request << "{\"state\": \"" << newState << "\"}";

    std::string response = sendRequest(request.str());
    return response.find("HTTP/1.1 200 OK") != std::string::npos;
}

std::string HomeAssistantAPI::getState(const std::string &entityId)
{
    std::ostringstream request;
    request << "GET /api/states/" << entityId << " HTTP/1.1\r\n";
    request << "Host: " << host << ":" << port << "\r\n\r\n";

    return sendRequest(request.str());
}

std::vector<std::string> HomeAssistantAPI::getEntityList()
{
    std::ostringstream request;
    request << "GET /api/states HTTP/1.1\r\n";
    request << "Host: " << host << ":" << port << "\r\n\r\n";

    std::string response = sendRequest(request.str());
    // Parsing JSON response (simplified example)
    std::vector<std::string> entities;
    size_t pos = 0;
    while ((pos = response.find("\"entity_id\":", pos)) != std::string::npos)
    {
        pos = response.find("\"", pos + 12);
        size_t endPos = response.find("\"", pos + 1);
        entities.push_back(response.substr(pos + 1, endPos - pos - 1));
        pos = endPos;
    }
    return entities;
}

std::map<std::string, std::string> HomeAssistantAPI::getEntityStates()
{
    std::ostringstream request;
    request << "GET /api/states HTTP/1.1\r\n";
    request << "Host: " << host << ":" << port << "\r\n\r\n";

    std::string response = sendRequest(request.str());
    // Parsing JSON response (simplified example)
    std::map<std::string, std::string> entityStates;
    size_t pos = 0;
    while ((pos = response.find("\"entity_id\":", pos)) != std::string::npos)
    {
        pos = response.find("\"", pos + 12);
        size_t endPos = response.find("\"", pos + 1);
        std::string entityId = response.substr(pos + 1, endPos - pos - 1);

        pos = response.find("\"state\":", endPos);
        pos = response.find("\"", pos + 8);
        endPos = response.find("\"", pos + 1);
        std::string state = response.substr(pos + 1, endPos - pos - 1);

        entityStates[entityId] = state;
        pos = endPos;
    }
    return entityStates;
}

bool HomeAssistantAPI::callService(const std::string &domain, const std::string &service, const std::string &entityId)
{
    std::ostringstream request;
    request << "POST /api/services/" << domain << "/" << service << " HTTP/1.1\r\n";
    request << "Host: " << host << ":" << port << "\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Content-Length: " << (entityId.length() + 22) << "\r\n\r\n";
    request << "{\"entity_id\": \"" << entityId << "\"}";

    std::string response = sendRequest(request.str());
    return response.find("HTTP/1.1 200 OK") != std::string::npos;
}
