#ifndef WIFI_H
#define WIFI_H

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

class wifiServer
{
public:
    wifiServer(int wifi_port);
    ~wifiServer();
    void run();
    void setupServerSocket();
    void bindSocket();
    void listenForClients();
    void acceptClient();
    void session(SoundData *soundData);
    void sendHttpResponse(int clientSd, const uint8_t *data, size_t length, const std::string &statusCode, const std::string &contentType);
    void closeSocket(int sd);

private:
    int wifi_port;
    int serverSd;
    sockaddr_in servAddr;
    std::vector<std::thread> clientThreads;
    std::mutex clientMutex;
};

class wifiClient
{
public:
    wifiClient(int wifi_port, const char *serverIp);
    ~wifiClient();
    void setupClientSocket();
    void connectToServer();
    void session(SoundData *soundData);
    void sendSoundData(const uint8_t *data, size_t length);
    void receiveResponse();
    void closeSocket(int sd);

private:
    const char *serverIp;
    int wifi_port;
    int serverSd;
    sockaddr_in servAddr;
};

#endif // WIFI_SERVER_H
