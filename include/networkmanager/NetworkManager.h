/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    10-04-2024
 * @Date updated    03-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the NetworkManager class and the SoundData struct
 */

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
#ifdef SERVER_BUILD
#include "ModelRunner.h"
#include "WhisperTranscriber.h"
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
    enum Protocol
    {
        TCP,
        UDP
    };

#if defined(BUILD_SERVER)
    NetworkManager(int port, char *serverIp, Protocol protocol, ModelRunner *nerModel, ModelRunner *classificationModel);
#else
    NetworkManager(int port, char *serverIp, Protocol protocol);
#endif
    ~NetworkManager();

    void runServer();
    void connectClient();

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
    bool isKnownClient(int clientSd);
    void addKnownClient(int clientSd);
    void processSoundData(const SoundData *inputData, uint8_t *outputData);

#if defined(BUILD_SERVER)
    ModelRunner *nerModel;            // Model for NER
    ModelRunner *classificationModel; // Model for Classification
    WhisperTranscriber transcriber;   // Whisper transcriber for live audio transcription
#endif
};

#endif // NETWORKMANAGER_H
