#include "Wifi.h"

wifiClient::wifiClient(int port) : port(port)
{
    setupClientSocket();
    bindSocket();
    listenForClients();
}

void wifiClient::session(int clientSd)
{
    char buffer[1024];
    memset(buffer, 0, 1024);
    int bytesReceived = recv(clientSd, buffer, 1024, 0);
    if (bytesReceived < 0)
    {
        std::cerr << "Failed to read data from client." << std::endl;
        return;
    }

    std::string request(buffer);
    if (request.find("GET /data ") != std::string::npos)
    {
        uint32_t dataLength;
        // Handle the GET /data request
    }
    else
    {
        sendHttpResponse(clientSd, reinterpret_cast<const uint8_t *>("Not Found"), 9, "404 Not Found", "text/plain");
    }
}

wifiClient::~wifiClient()
{
    closeSocket(serverSd);
}

void wifiClient::run()
{
    sockaddr_in newSockAddr;
    int newSd;
    acceptClient(newSockAddr, newSd);
    session(newSd);
    closeSocket(newSd);
}

void wifiClient::setupClientSocket()
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

void wifiClient::bindSocket()
{
    int bindStatus = bind(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (bindStatus < 0)
    {
        std::cerr << "Error binding socket to local address" << std::endl;
        exit(0);
    }
}

void wifiClient::listenForClients()
{
    listen(serverSd, 5); // Listen for up to 5 requests at a time
    std::cout << "Waiting for a client to connect..." << std::endl;
}

void wifiClient::searchForHost(const char *hostname)
{
    struct hostent *host = gethostbyname(hostname);
    if (host == NULL)
    {
        std::cerr << "Error searching for host" << std::endl;
        exit(0);
    }

    struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
    for (int i = 0; addr_list[i] != NULL; i++)
    {
        servAddr.sin_addr = *addr_list[i];
        // Try to connect to the host with the given IP address and port
        if (connectToServer())
        {
            std::cout << "Connected to server at " << inet_ntoa(servAddr.sin_addr) << std::endl;
            return;
        }
    }
    std::cerr << "Failed to connect to any host" << std::endl;
    exit(0);
}

bool wifiClient::connectToServer()
{
    int connectionStatus = connect(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    return connectionStatus >= 0;
}

void wifiClient::acceptClient(sockaddr_in &newSockAddr, int &newSd)
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

void wifiClient::closeSocket(int sd)
{
    close(sd);
}

void wifiClient::sendHttpResponse(int clientSd, const uint8_t *data, size_t length, const std::string &statusCode, const std::string &contentType)
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
