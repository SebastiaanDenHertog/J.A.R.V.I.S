#include "Wifi.h"

wifiClient::wifiClient(int port, const char *serverIp) : serverIp(serverIp), port(port)
{
    setupClientSocket();
    connectToServer();
}

wifiClient::~wifiClient()
{
    closeSocket(serverSd);
}

void wifiClient::setupClientSocket()
{
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSd < 0)
    {
        std::cerr << "Error establishing the client socket" << std::endl;
    }
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
}

void wifiClient::connectToServer()
{
    servAddr.sin_addr.s_addr = inet_addr(serverIp);
    servAddr.sin_port = htons(port); // Add this line to set the port

    int connectionStatus = connect(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (connectionStatus < 0)
    {
        std::cerr << "Error connecting to server" << std::endl;
    }
}

void wifiClient::sendSoundData(const uint8_t *data, size_t length)
{
    std::ostringstream request;
    request << "POST /sound HTTP/1.1\r\n";
    request << "Content-Length: " << length << "\r\n";
    request << "Content-Type: application/octet-stream\r\n\r\n";

    send(serverSd, request.str().c_str(), request.str().length(), 0);
    send(serverSd, data, length, 0);
}

void wifiClient::receiveResponse()
{
    char buffer[1024];
    memset(buffer, 0, 1024);
    int bytesReceived = recv(serverSd, buffer, 1024, 0);
    if (bytesReceived < 0)
    {
        std::cerr << "Failed to read data from server." << std::endl;
        return;
    }

    std::cout << "Received response from server: " << std::string(buffer, bytesReceived) << std::endl;
}

void wifiClient::closeSocket(int sd)
{
    close(sd);
}
