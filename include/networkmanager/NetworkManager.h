#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include <mutex>

#ifdef SERVER_MODE
#include "model_runner.h"
#endif

struct SoundData
{
    uint8_t *data;
    size_t length;
    int clientSd;

    SoundData(size_t len, int sd) : length(len), clientSd(sd)
    {
        data = new uint8_t[length];
    }

    ~SoundData()
    {
        delete[] data;
    }
};
struct soundData
{
    uint8_t *audioData;
    uint32_t dataLength;

    soundData() : audioData(nullptr), dataLength(0) {}

    ~soundData()
    {
        delete[] audioData;
    }

    void update(uint8_t *newData, uint32_t newLength)
    {
        delete[] audioData;
        audioData = new uint8_t[newLength];
        std::copy(newData, newData + newLength, audioData);
        dataLength = newLength;
    }
};

class NetworkManager
{
public:
    NetworkManager(int port, const char *serverIp = nullptr, bool isServer = true);
    ~NetworkManager();
    void runServer();
    void connectClient();
    void sendSoundData(const uint8_t *data, size_t length);
    void receiveResponse();

private:
    void setupServerSocket();
    void bindSocket();
    void listenForClients();
    void acceptClient();
    void setupClientSocket();
    void connectToServer();
    void session(int clientSd);
    void sendHttpResponse(int clientSd, const uint8_t *data, size_t length, const std::string &statusCode, const std::string &contentType);
    void closeSocket(int sd);
    static void processSoundData(const SoundData *inputData, uint8_t *outputData);

    int port;
    const char *serverIp;
    int serverSd;
    sockaddr_in servAddr;
    std::vector<std::thread> clientThreads;
    std::mutex clientMutex;
    bool isServer; // Flag to determine if the instance should perform model processing
};

#endif // NETWORK_MANAGER_H
