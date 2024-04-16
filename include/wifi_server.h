#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

class wifiServer
{
public:
    wifiServer(int port);
    ~wifiServer();
    void run();

private:
    int serverSd;
    sockaddr_in servAddr;
    int port;

    void setupServerSocket();
    void bindSocket();
    void listenForClients();
    void acceptClient(sockaddr_in &newSockAddr, int &newSd);
    void session(int &clientSd);
    void closeSocket(int sd);
};

#endif // WIFI_SERVER_H
