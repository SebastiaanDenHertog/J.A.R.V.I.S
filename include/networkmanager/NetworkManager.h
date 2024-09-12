#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <sstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#if BUILD_SERVER == ON || BUILD_FULL == ON
    #include "ModelRunner.h"
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

class NetworkManager
{
public:
    enum Protocol
    {
        TCP,
        UDP
    };

#if BUILD_SERVER == ON || BUILD_FULL == ON
    NetworkManager(int port, const char *serverIp, Protocol protocol,ModelRunner *nerModel, ModelRunner *classificationModel);
#else
    NetworkManager(int port, const char *serverIp, Protocol protocol);
#endif
  
    ~NetworkManager();

    void runServer();
    void connectClient();
    bool isConnectedToSpecialServer() const;

    void sendSoundData(const uint8_t *data, size_t length);
    void receiveResponse();

    void send(int sd, const char *data, size_t length, int flags);
    void send(int sd, const uint8_t *data, size_t length, int flags);
    void sendToUDP(const uint8_t *data, size_t length);
    int recv(int sd, char *buffer, size_t length, int flags);
    int recvFromUDP(uint8_t *buffer, size_t length);
    int getServerSocket() const;

private:
    int port;
    const char *serverIp;
    int serverSd;
    int udpSd;
    sockaddr_in servAddr;
    sockaddr_in clientAddrUDP;
    socklen_t clientAddrUDPSize;
    bool connectedToSpecialServer;
    Protocol protocol;
    std::vector<std::thread> clientThreads;
    std::mutex clientMutex;
    std::unordered_set<int> knownClients;

    void setupServerSocket();
    void setupUDPSocket();
    void bindSocket();
    void listenForClients();
    void acceptClient();
    void setupClientSocket();
    void connectToServer();
    void session(int clientSd);
    void sendHttpResponse(int clientSd, const uint8_t *data, size_t length, const std::string &statusCode, const std::string &contentType);
    void closeSocket(int sd);
    void processSoundData(const SoundData *inputData, uint8_t *outputData);
    bool isKnownClient(int clientSd);
    void addKnownClient(int clientSd);

#if BUILD_SERVER == ON || BUILD_FULL == ON
    ModelRunner *nerModel;  // Model for NER
    ModelRunner *classificationModel; // Model for Classification
#endif
};

#endif // NETWORKMANAGER_H
