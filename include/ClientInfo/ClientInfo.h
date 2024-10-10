/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    29-07-2024
 * @Date updated    21-09-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the ClientInfo class
 **/

#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include <string>
#include <vector>

class ClientInfo
{
public:
    // Constructor
    ClientInfo(const std::string &identifier, const std::string &ipAddress, const int port, const std::vector<std::string> &musicOutputs);

    // Getters
    std::string getIpAddress() const;
    int getPort() const;
    std::vector<std::string> getMusicOutputs() const;
    std::vector<std::string> getVideoOutputs() const;
    std::string getIdentifier() const;

    // Setters
    void setIpAddress(const std::string &ipAddress);
    void setPort(const int &port);
    void setVideoOutputs(const std::vector<std::string> &videoOutputs);
    void setVideoOutput(const std::string &output);
    void setMusicOutputs(const std::vector<std::string> &musicOutputs);
    void setMusicOutput(const std::string &output);
    void setIdentifier(const std::string &identifier);

private:
    std::string identifier;
    std::string ipAddress;
    int port;
    std::vector<std::string> musicOutputs;
    std::vector<std::string> videoOutputs;
};

#endif // CLIENTINFO_H
