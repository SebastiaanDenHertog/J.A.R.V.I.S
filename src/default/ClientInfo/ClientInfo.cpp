/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    29-07-2024
 * @Date updated    21-09-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the ClientInfo class
 **/

#include "ClientInfo.h"

/**
 * @brief Constructor for ClientInfo.
 * @param identifier The identifier of the client.
 * @param ipAddress The IP address of the client.
 * @param port The port of the client.
 * @param musicOutputs The music outputs of the client.
 */
ClientInfo::ClientInfo(const std::string &identifier, const std::string &ipAddress, const int port, const std::vector<std::string> &musicOutputs)
    : identifier(identifier), ipAddress(ipAddress), port(port), musicOutputs(musicOutputs) {}

/**
 * @brief Get the IP address of the client.
 * @return The IP address of the client.
 */
std::string ClientInfo::getIpAddress() const
{
    return ipAddress;
}

/**
 * @brief Get the port of the client.
 * @return The port of the client.
 */

int ClientInfo::getPort() const
{
    return port;
}

/**
 * @brief Get the music outputs of the client.
 * @return The music outputs of the client.
 */

std::vector<std::string> ClientInfo::getMusicOutputs() const
{
    return musicOutputs;
}

/**
 * @brief Get the video outputs of the client.
 * @return The video outputs of the client.
 */

std::vector<std::string> ClientInfo::getVideoOutputs() const
{
    return videoOutputs;
}

/**
 * @brief Get the identifier of the client.
 * @return The identifier of the client.
 */

std::string ClientInfo::getIdentifier() const
{
    return identifier;
}

/**
 * @brief Set the IP address of the client.
 * @param ipAddress The IP address to set.
 */

void ClientInfo::setIpAddress(const std::string &ipAddress)
{
    this->ipAddress = ipAddress;
}

/**
 * @brief Set the port of the client.
 * @param port The port to set.
 */

void ClientInfo::setPort(const int &Port)
{
    this->port = port;
}

/**
 * @brief Set the video outputs of the client.
 * @param videoOutputs The video outputs to set.
 */

void ClientInfo::setVideoOutputs(const std::vector<std::string> &videoOutputs)
{
    this->videoOutputs = videoOutputs;
}

/**
 * @brief Set the video output of the client.
 * @param output The video output to set.
 */

void ClientInfo::setVideoOutput(const std::string &output)
{
    this->videoOutputs.push_back(output);
}

/**
 * @brief Set the music outputs of the client.
 * @param musicOutputs The music outputs to set.
 */

void ClientInfo::setMusicOutputs(const std::vector<std::string> &musicOutputs)
{
    this->musicOutputs = musicOutputs;
}

/**
 * @brief Set the music output of the client.
 * @param output The music output to set.
 */

void ClientInfo::setMusicOutput(const std::string &output)
{
    this->musicOutputs.push_back(output);
}

/**
 * @brief Set the identifier of the client.
 * @param identifier The identifier to set.
 */

void ClientInfo::setIdentifier(const std::string &identifier)
{
    this->identifier = identifier;
}
