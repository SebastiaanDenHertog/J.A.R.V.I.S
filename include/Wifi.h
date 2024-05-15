#ifndef WIFI_H
#define WIFI_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <cstring>
#include "ReSpeaker.h"

class wifiServer
{
public:
    wifiServer(int port, ReSpeaker &respeaker);
    ~wifiServer();
    void run();
    void setupServerSocket();
    void bindSocket();
    void listenForClients();
    void acceptClient(sockaddr_in &newSockAddr, int &newSd);
    void session(int clientSd);
    void sendHttpResponse(int clientSd, const uint8_t *data, size_t length, const std::string &statusCode, const std::string &contentType);
    void closeSocket(int sd);

private:
    ReSpeaker &respeaker;
    int port;
    int serverSd;
    sockaddr_in servAddr;
};

#endif // WIFI_SERVER_H
