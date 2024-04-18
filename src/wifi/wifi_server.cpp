#include "wifi_server.h"

wifiServer::wifiServer(int port, ReSpeaker &respeaker) : port(port), respeaker(respeaker)
{
    setupServerSocket();
    bindSocket();
    listenForClients();
}

void wifiServer::session(int clientSd)
{
    char buffer[1024];
    memset(buffer, 0, 1024);
    int bytesReceived = recv(clientSd, buffer, 1024, 0);
    if (bytesReceived < 0)
    {
        std::cerr << "Failed to read data from client." << std::endl;
        return;
    }

    std::string request(buffer);
    if (request.find("GET /data ") != std::string::npos)
    {
        uint32_t dataLength;
        uint8_t *audioData = respeaker.startCaptureAndGetAudioData(dataLength);
        if (audioData != nullptr)
        {
            sendHttpResponse(clientSd, audioData, dataLength, "200 OK", "application/octet-stream");
            delete[] audioData;
        }
        else
        {
            sendHttpResponse(clientSd, reinterpret_cast<const uint8_t *>("No data"), 7, "404 Not Found", "text/plain");
        }
        respeaker.stopCapture();
    }
    else
    {
        sendHttpResponse(clientSd, reinterpret_cast<const uint8_t *>("Not Found"), 9, "404 Not Found", "text/plain");
    }
}

// Destructor
wifiServer::~wifiServer()
{
    closeSocket(serverSd);
}

void wifiServer::run()
{
    sockaddr_in newSockAddr;
    int newSd;
    acceptClient(newSockAddr, newSd);
    session(newSd);
    closeSocket(newSd);
}

void wifiServer::setupServerSocket()
{
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSd < 0)
    {
        std::cerr << "Error establishing the server socket" << std::endl;
        exit(0);
    }
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
}

void wifiServer::bindSocket()
{
    int bindStatus = bind(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (bindStatus < 0)
    {
        std::cerr << "Error binding socket to local address" << std::endl;
        exit(0);
    }
}

void wifiServer::listenForClients()
{
    listen(serverSd, 5); // Listen for up to 5 requests at a time
    std::cout << "Waiting for a client to connect..." << std::endl;
}

void wifiServer::acceptClient(sockaddr_in &newSockAddr, int &newSd)
{
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if (newSd < 0)
    {
        std::cerr << "Error accepting request from client!" << std::endl;
        exit(1);
    }
    std::cout << "Connected with client!" << std::endl;
}

void wifiServer::closeSocket(int sd)
{
    close(sd);
}

void wifiServer::sendHttpResponse(int clientSd, const uint8_t *data, size_t length, const std::string &statusCode, const std::string &contentType)
{
    std::ostringstream httpResponse;
    httpResponse << "HTTP/1.1 " << statusCode << "\r\n";
    httpResponse << "Content-Type: " << contentType << "\r\n";
    httpResponse << "Content-Length: " << length << "\r\n";
    httpResponse << "Connection: close\r\n\r\n";
    send(clientSd, httpResponse.str().c_str(), httpResponse.str().length(), 0);

    if (data != nullptr && length > 0)
    {
        send(clientSd, data, length, 0);
    }
}

