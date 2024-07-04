#include "ClientInfo.h"

// Constructor
ClientInfo::ClientInfo(const std::string &identifier, const std::string &ipAddress, const int port, const std::vector<std::string> &musicOutputs)
    : identifier(identifier), ipAddress(ipAddress), port(port), musicOutputs(musicOutputs) {}

// Getters
std::string ClientInfo::getIpAddress() const
{
    return ipAddress;
}

int ClientInfo::getPort() const
{
    return port;
}

std::vector<std::string> ClientInfo::getMusicOutputs() const
{
    return musicOutputs;
}

std::vector<std::string> ClientInfo::getVideoOutputs() const
{
    return videoOutputs;
}

std::string ClientInfo::getIdentifier() const
{
    return identifier;
}

// Setters
void ClientInfo::setIpAddress(const std::string &ipAddress)
{
    this->ipAddress = ipAddress;
}

void ClientInfo::setPort(const int &Port)
{
    this->port = port;
}

void ClientInfo::setVideoOutputs(const std::vector<std::string> &videoOutputs)
{
    this->videoOutputs = videoOutputs;
}

void ClientInfo::setVideoOutput(const std::string &output)
{
    this->videoOutputs.push_back(output);
}

void ClientInfo::setMusicOutputs(const std::vector<std::string> &musicOutputs)
{
    this->musicOutputs = musicOutputs;
}

void ClientInfo::setMusicOutput(const std::string &output)
{
    this->musicOutputs.push_back(output);
}

void ClientInfo::setIdentifier(const std::string &identifier)
{
    this->identifier = identifier;
}
