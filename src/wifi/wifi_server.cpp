#include "wifi_server.h"

// Constructor
Server::Server(int port) : port(port), serverSd(-1)
{
    setupServerSocket();
    bindSocket();
    listenForClients();
}

// Destructor
Server::~Server()
{
    closeSocket(serverSd);
}

void Server::run()
{
    sockaddr_in newSockAddr;
    int newSd;
    acceptClient(newSockAddr, newSd);
    session(newSd);
    closeSocket(newSd);
}

void Server::setupServerSocket()
{
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSd < 0)
    {
        std::cerr << "Error establishing the server socket" << std::endl;
        exit(0);
    }
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
}

void Server::bindSocket()
{
    int bindStatus = bind(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (bindStatus < 0)
    {
        std::cerr << "Error binding socket to local address" << std::endl;
        exit(0);
    }
}

void Server::listenForClients()
{
    listen(serverSd, 5); // Listen for up to 5 requests at a time
    std::cout << "Waiting for a client to connect..." << std::endl;
}

void Server::acceptClient(sockaddr_in &newSockAddr, int &newSd)
{
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if (newSd < 0)
    {
        std::cerr << "Error accepting request from client!" << std::endl;
        exit(1);
    }
    std::cout << "Connected with client!" << std::endl;
}

void Server::session(int &clientSd)
{
    // Implementation of session handling with client
}

void Server::closeSocket(int sd)
{
    close(sd);
}
