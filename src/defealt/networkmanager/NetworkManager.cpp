#include "NetworkManager.h"
#ifdef SERVER_MODE
#include "model_runner.h"
#endif
#include <cerrno>
#include <cstring>
#include <iostream>

NetworkManager::NetworkManager(int port, const char *serverIp, bool isServer)
    : port(port), serverIp(serverIp), serverSd(-1), isServer(isServer), connectedToSpecialServer(false)
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

NetworkManager::~NetworkManager()
{
    closeSocket(serverSd);
    for (auto &th : clientThreads)
    {
        if (th.joinable())
        {
            th.join();
        }
    }
}

void NetworkManager::runServer()
{
    while (true)
    {
        acceptClient();
    }
}

void NetworkManager::connectClient()
{
    connectToServer();

    // Check if connected to the special server
    if (std::strcmp(serverIp, "192.168.1.100") == 0) // Replace with your special server IP
    {
        connectedToSpecialServer = true;
    }
}

bool NetworkManager::isConnectedToSpecialServer() const
{
    return connectedToSpecialServer;
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
        perror("setsockopt");
        exit(1);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
    std::cout << "Server socket created with descriptor: " << serverSd << std::endl;
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

    int connectionStatus = -1;
    while (connectionStatus < 0)
    {
        connectionStatus = connect(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));
        if (connectionStatus < 0)
        {
            perror("Error connecting to server. Retrying in 5 seconds...");
            sleep(5);
        }
    }
    std::cout << "Successfully connected to the server." << std::endl;
}

void NetworkManager::sendSoundData(const uint8_t *data, size_t length)
{
    std::ostringstream request;
    request << "POST /sound HTTP/1.1\r\n";
    request << "Content-Length: " << length << "\r\n";
    request << "Content-Type: application/octet-stream\r\n\r\n";

    send(serverSd, request.str().c_str(), request.str().length(), 0);
    send(serverSd, data, length, 0);
}

void NetworkManager::receiveResponse()
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

    // Parse the HTTP request to extract the sound data
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

    // Receive the rest of the data if it wasn't all received in the first recv call
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

    if (isServer)
    {
        // Process the sound data only if this is the server
#ifdef SERVER_MODE
        ModelRunner modelRunner("models/whisper_english.tflite");
        modelRunner.modelsLogic(&soundData);

        uint8_t *processedData = new uint8_t[soundData.length];
        processSoundData(&soundData, processedData);

        // Send the processed data back to the client
        sendHttpResponse(clientSd, processedData, soundData.length, "200 OK", "application/octet-stream");
        delete[] processedData;
#endif
    }
    else
    {
        // Handle client-specific logic here (if any)
        std::cout << "Client received data of length: " << soundData.length << std::endl;
    }

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
