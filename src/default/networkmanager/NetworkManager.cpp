#include "NetworkManager.h"
#include <cerrno>
#include <cstring>
#include <iostream>

#if defined(BUILD_FULL) || defined(BUILD_SERVER)

NetworkManager::NetworkManager(int port, char* serverIp, Protocol protocol, ModelRunner *nerModel, ModelRunner *classificationModel)
    : port(port), serverIp(serverIp), serverSd(-1), udpSd(-1), connectedToSpecialServer(false), protocol(protocol), clientAddrUDPSize(sizeof(clientAddrUDP)), nerModel(nerModel), classificationModel(classificationModel)
{
    std::cout << "Server IP: " << (serverIp ? serverIp : "None") << std::endl;
    std::cout << "Server Port: " << port << std::endl;

    if (protocol == TCP)
    {
        if (serverIp == nullptr)
        {
            setupServerSocket();
            bindSocket();
            listenForClients();
        }
        else
        {
            setupClientSocket();
        }
    }
    else if (protocol == UDP)
    {
        setupUDPSocket();
    }
}
#else
NetworkManager::NetworkManager(int port, char* serverIp, Protocol protocol)
    : port(port), serverIp(serverIp), serverSd(-1), udpSd(-1), connectedToSpecialServer(false), protocol(protocol), clientAddrUDPSize(sizeof(clientAddrUDP))
{
    std::cout << "Server IP: " << (serverIp ? serverIp : "None") << std::endl;
    std::cout << "Server Port: " << port << std::endl;

    if (protocol == TCP)
    {
        if (serverIp == nullptr)
        {
            setupServerSocket();
            bindSocket();
            listenForClients();
        }
        else
        {
            setupClientSocket();
        }
    }
    else if (protocol == UDP)
    {
        setupUDPSocket();
    }
}
#endif

NetworkManager::~NetworkManager()
{
    closeSocket(serverSd);
    closeSocket(udpSd);
    for (auto &th : clientThreads)
    {
        if (th.joinable())
        {
            th.join();
        }
    }
}

void NetworkManager::setupServerSocket()
{
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSd < 0)
    {
        perror("Error establishing the server socket");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        exit(1);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    std::cout << "Server socket created" << std::endl;
}

void NetworkManager::setupUDPSocket()
{
    udpSd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSd < 0)
    {
        perror("Error establishing the UDP socket");
        exit(1);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
    std::cout << "UDP Socket created" << std::endl;

    if (bind(udpSd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error binding UDP socket");
        exit(1);
    }
}

void NetworkManager::runServer()
{
    if (protocol == TCP)
    {
        std::cout << "TCP connection setup." << std::endl;
        while (true)
        {
            acceptClient();
        }
    }
    else if (protocol == UDP)
    {
        std::cout << "UDP connection setup." << std::endl;
        while (true)
        {
            uint8_t buffer[1024];
            int bytesReceived = recvFromUDP(buffer, sizeof(buffer));
            if (bytesReceived > 0)
            {
                std::cout << "Received UDP data: " << std::string((char *)buffer, bytesReceived) << std::endl;
            }
        }
    }
}

void NetworkManager::sendToUDP(const uint8_t *data, size_t length)
{
    sendto(udpSd, data, length, 0, (struct sockaddr *)&clientAddrUDP, clientAddrUDPSize);
}

int NetworkManager::recvFromUDP(uint8_t *buffer, size_t length)
{
    return recvfrom(udpSd, buffer, length, 0, (struct sockaddr *)&clientAddrUDP, &clientAddrUDPSize);
}

void NetworkManager::connectClient()
{
    if (protocol == TCP)
    {
        std::cout << "TCP connection setup." << std::endl;
        connectToServer();

    }
    else if (protocol == UDP)
    {
        std::cout << "UDP connection setup." << std::endl;
    }
}

void NetworkManager::setupClientSocket()
{
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSd < 0)
    {
        perror("Error establishing the client socket");
        exit(0);
    }
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
}

void NetworkManager::connectToServer()
{
    servAddr.sin_addr.s_addr = inet_addr(serverIp);
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    std::cout << "Client trying to connect to server at IP: " << serverIp << " on port: " << port << std::endl;

    int connectionStatus = -1;
    while (connectionStatus < 0)
    {
        connectionStatus = connect(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));
        if (connectionStatus < 0)
        {
            std::cerr << "Error connecting to server. Error: " << strerror(errno) << std::endl;
            std::cerr << "Retrying connection in 5 seconds..." << std::endl;
            sleep(5);
        }
    }
    std::cout << "Successfully connected to the server!" << std::endl;
}


void NetworkManager::sendSoundData(const uint8_t *data, size_t length)
{
    if (protocol == TCP)
    {
        std::ostringstream request;
        request << "POST /sound HTTP/1.1\r\n";
        request << "Content-Length: " << length << "\r\n";
        request << "Content-Type: application/octet-stream\r\n\r\n";

        send(serverSd, request.str().c_str(), request.str().length(), 0);
        send(serverSd, data, length, 0);
    }
    else if (protocol == UDP)
    {
        sendToUDP(data, length);
    }
}

void NetworkManager::receiveResponse()
{
    if (protocol == TCP)
    {
        char buffer[1024];
        memset(buffer, 0, 1024);
        int bytesReceived = recv(serverSd, buffer, 1024, 0);
        if (bytesReceived < 0)
        {
            perror("Failed to read data from server");
            return;
        }

        std::cout << "Received response from server: " << std::string(buffer, bytesReceived) << std::endl;
    }
    else if (protocol == UDP)
    {
        uint8_t buffer[1024];
        int bytesReceived = recvFromUDP(buffer, sizeof(buffer));
        if (bytesReceived > 0)
        {
            std::cout << "Received UDP response: " << std::string((char *)buffer, bytesReceived) << std::endl;
        }
    }
}

void NetworkManager::bindSocket()
{
    if (serverSd < 0)
    {
        std::cerr << "Invalid server socket descriptor" << std::endl;
        return;
    }

    int bindStatus = bind(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (bindStatus < 0)
    {
        perror("Error binding socket to local address");
        exit(1);
    }
    std::cout << "Server socket bound to address" << std::endl;
}

void NetworkManager::listenForClients()
{
    if (serverSd < 0)
    {
        std::cerr << "Invalid server socket descriptor" << std::endl;
        return;
    }

    std::cout << "Server running on port " << port << std::endl;
    if (listen(serverSd, 5) < 0)
    {
        perror("Error listening on socket");
        exit(1);
    }
    std::cout << "Server is now listening for clients..." << std::endl;
}


void NetworkManager::acceptClient()
{
    if (serverSd < 0)
    {
        std::cerr << "Invalid server socket descriptor" << std::endl;
        return;
    }

    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    std::cout << "Waiting for client connection..." << std::endl;
    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if (newSd < 0)
    {
        perror("Error accepting client connection");
        return;
    }
    std::cout << "Connected with client! New socket descriptor: " << newSd << std::endl;

    std::lock_guard<std::mutex> guard(clientMutex);
    clientThreads.push_back(std::thread(&NetworkManager::session, this, newSd));
}

void NetworkManager::session(int clientSd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytesReceived = recv(clientSd, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0)
    {
        perror("Failed to read data from client");
        closeSocket(clientSd);
        return;
    }

    std::string request(buffer);
    size_t contentLengthPos = request.find("Content-Length: ");
    if (contentLengthPos == std::string::npos)
    {
        sendHttpResponse(clientSd, reinterpret_cast<const uint8_t *>("Bad Request"), 11, "400 Bad Request", "text/plain");
        closeSocket(clientSd);
        return;
    }

    contentLengthPos += 16; // Move past "Content-Length: "
    size_t endOfContentLength = request.find("\r\n", contentLengthPos);
    size_t contentLength = std::stoi(request.substr(contentLengthPos, endOfContentLength - contentLengthPos));

    size_t headerEnd = request.find("\r\n\r\n");
    size_t dataStart = headerEnd + 4;

    SoundData soundData(contentLength, clientSd);

    if (dataStart < request.size())
    {
        std::memcpy(soundData.data, buffer + dataStart, request.size() - dataStart);
    }

    size_t bytesRemaining = contentLength - (request.size() - dataStart);
    size_t offset = request.size() - dataStart;
    while (bytesRemaining > 0)
    {
        int bytesReceived = recv(clientSd, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
        {
            perror("Failed to read remaining data from client");
            closeSocket(clientSd);
            return;
        }
        std::memcpy(soundData.data + offset, buffer, bytesReceived);
        offset += bytesReceived;
        bytesRemaining -= bytesReceived;
    }

    // Forward data to ModelRunner and get the result
    uint8_t processedData[soundData.length];
    processSoundData(&soundData, processedData);

    // Send the processed data back to the client
    sendHttpResponse(clientSd, processedData, soundData.length, "200 OK", "application/octet-stream");

    closeSocket(clientSd);
}

void NetworkManager::sendHttpResponse(int clientSd, const uint8_t *data, size_t length, const std::string &statusCode, const std::string &contentType)
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

void NetworkManager::closeSocket(int sd)
{
    if (sd >= 0)
    {
        std::cout << "Closing socket descriptor: " << sd << std::endl;
        close(sd);
    }
}

void NetworkManager::processSoundData(const SoundData *inputData, uint8_t *outputData)
{
    for (size_t i = 0; i < inputData->length; ++i)
    {
        outputData[i] = ~inputData->data[i]; // Example processing: inverting the data
    }
}

bool NetworkManager::isKnownClient(int clientSd)
{
    std::lock_guard<std::mutex> guard(clientMutex);
    return knownClients.find(clientSd) != knownClients.end();
}

void NetworkManager::addKnownClient(int clientSd)
{
    std::lock_guard<std::mutex> guard(clientMutex);
    knownClients.insert(clientSd);
}

int NetworkManager::getServerSocket() const
{
    return serverSd;
}

void NetworkManager::send(int sd, const char *data, size_t length, int flags)
{
    ::send(sd, data, length, flags);
}

void NetworkManager::send(int sd, const uint8_t *data, size_t length, int flags)
{
    ::send(sd, reinterpret_cast<const char *>(data), length, flags);
}

int NetworkManager::recv(int sd, char *buffer, size_t length, int flags)
{
    return ::recv(sd, buffer, length, flags);
}

