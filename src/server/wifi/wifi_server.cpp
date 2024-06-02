#include "Wifi.h"
#include "model_runner.h"

// Helper function to process sound data (example: inverting the data)
void processSoundData(const SoundData *inputData, uint8_t *outputData)
{
    for (size_t i = 0; i < inputData->length; ++i)
    {
        outputData[i] = ~inputData->data[i]; // Example processing: inverting the data
    }
}

wifiServer::wifiServer(int wifi_port) : wifi_port(wifi_port)
{
    setupServerSocket();
    bindSocket();
    listenForClients();
}

wifiServer::~wifiServer()
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

void wifiServer::run()
{
    while (true)
    {
        acceptClient();
    }
}

void wifiServer::setupServerSocket()
{
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSd < 0)
    {
        std::cerr << "Error establishing the WifiServer socket" << std::endl;
        exit(0);
    }
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(wifi_port);
    std::cout << "WifiServer socket created" << std::endl;
}

void wifiServer::bindSocket()
{
    int bindStatus = bind(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (bindStatus < 0)
    {
        std::cerr << "Error binding socket to local address" << std::endl;
    }
    std::cout << "WifiServer socket bound to address" << std::endl;
}

void wifiServer::listenForClients()
{
    std::cout << "WifiServer running on port " << std::to_string(wifi_port) << std::endl;
    std::cout << "Waiting for a client to connect..." << std::endl;
    listen(serverSd, 5); // Listen for up to 5 requests at a time
}

void wifiServer::acceptClient()
{
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if (newSd < 0)
    {
        std::cerr << "Error accepting client connection" << std::endl;
        return;
    }
    std::cout << "Connected with client!" << std::endl;

    std::lock_guard<std::mutex> guard(clientMutex);
    clientThreads.push_back(std::thread([this, newSd]
                                        {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recv(newSd, buffer, sizeof(buffer), 0);
        if (bytesReceived < 0)
        {
            std::cerr << "Failed to read data from client." << std::endl;
            closeSocket(newSd);
            return;
        }

        // Parse the HTTP request to extract the sound data
        std::string request(buffer);
        size_t contentLengthPos = request.find("Content-Length: ");
        if (contentLengthPos == std::string::npos)
        {
            sendHttpResponse(newSd, reinterpret_cast<const uint8_t *>("Bad Request"), 11, "400 Bad Request", "text/plain");
            closeSocket(newSd);
            return;
        }

        contentLengthPos += 16; // Move past "Content-Length: "
        size_t endOfContentLength = request.find("\r\n", contentLengthPos);
        size_t contentLength = std::stoi(request.substr(contentLengthPos, endOfContentLength - contentLengthPos));

        size_t headerEnd = request.find("\r\n\r\n");
        size_t dataStart = headerEnd + 4;

        SoundData *soundData = new SoundData(contentLength, newSd);
        
        if (dataStart < request.size())
        {
            std::memcpy(soundData->data, buffer + dataStart, request.size() - dataStart);
        }

        // Receive the rest of the data if it wasn't all received in the first recv call
        size_t bytesRemaining = contentLength - (request.size() - dataStart);
        size_t offset = request.size() - dataStart;
        while (bytesRemaining > 0)
        {
            bytesReceived = recv(newSd, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0)
            {
                std::cerr << "Failed to read remaining data from client." << std::endl;
                closeSocket(newSd);
                delete soundData;
                return;
            }
            std::memcpy(soundData->data + offset, buffer, bytesReceived);
            offset += bytesReceived;
            bytesRemaining -= bytesReceived;
        }

        // Process the sound data
        session(soundData);

        // Clean up
        delete soundData;
        closeSocket(newSd); }));
}

void wifiServer::session(SoundData *soundData)
{
    ModelRunner ModelRunner("models/whisper_english.tflite");
    ModelRunner.modelsLogic(soundData);

    uint8_t *processedData = new uint8_t[soundData->length];

    // Send the processed data back to the client
    sendHttpResponse(soundData->clientSd, processedData, soundData->length, "200 OK", "application/octet-stream");

    delete[] processedData;
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