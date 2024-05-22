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

class wifiServer
{
public:
    wifiServer(int port);
    ~wifiServer();
    void run();
    void setupServerSocket();
    void bindSocket();
    void listenForClients();
    void acceptClient(sockaddr_in &newSockAddr, int &newSd);
    void session(int clientSd);
    void sendHttpResponse(int clientSd, const uint8_t *data, size_t length, const std::string &statusCode, const std::string &contentType);
    void closeSocket(int sd);
    vac

private:
    int port;
    int serverSd;
    sockaddr_in servAddr;

};

#endif // WIFI_SERVER_H

class wifiClient
{
public:
    wifiClient(int port);
    ~wifiClient();
    void run();
    void setupClientSocket();
    void bindSocket();
    void listenForClients();
    void acceptClient(sockaddr_in &newSockAddr, int &newSd);
    void session(int clientSd);
    void sendHttpResponse(int clientSd, const uint8_t *data, size_t length, const std::string &statusCode, const std::string &contentType);
    void closeSocket(int sd);
    void searchForHost(const char *hostname);
    bool connectToServer();
    int serverSd;

private:
    int port;
    sockaddr_in servAddr;
};