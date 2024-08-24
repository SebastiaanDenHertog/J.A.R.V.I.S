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

class ModelRunner; // Forward declaration

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
    NetworkManager(int port, const char *serverIp, ModelRunner *nerModel, ModelRunner *classificationModel);
    ~NetworkManager();

    void runServer();
    void connectClient();
    bool isConnectedToSpecialServer() const;

    void sendSoundData(const uint8_t *data, size_t length);
    void receiveResponse();

    void send(int sd, const char *data, size_t length, int flags);
    void send(int sd, const uint8_t *data, size_t length, int flags);
    int recv(int sd, char *buffer, size_t length, int flags);
    int getServerSocket() const;

private:
    int port;
    const char *serverIp;
    int serverSd;
    sockaddr_in servAddr;
    bool connectedToSpecialServer;
    std::vector<std::thread> clientThreads;
    std::mutex clientMutex;
    std::unordered_set<int> knownClients;

    ModelRunner *nerModel; // Pointer to ModelRunner
    ModelRunner *classificationModel;

    void setupServerSocket();
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
};

#endif // NETWORKMANAGER_H