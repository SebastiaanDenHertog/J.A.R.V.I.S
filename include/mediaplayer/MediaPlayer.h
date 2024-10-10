/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    24-05-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the MediaPlayer class
 **/

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <string>
#include <iostream>
#include <vector>
#include "ClientInfo.h"

class MediaPlayer
{
public:
    MediaPlayer();
    virtual ~MediaPlayer();

    // Initialize the media player with different protocols
    bool initializeBluetooth();
    bool initializeWiFi();
    bool initializeAUX();

    bool setoutput(ClientInfo device, std::string output);

    std::string FindSong(std::vector<std::vector<std::string>> entities);

    // Media control methods
    bool play(const std::string &mediaPath);
    bool pause();
    bool stop();

    // Method to select the playback protocol
    bool setProtocol(const std::string &protocol);

private:
    std::string currentProtocol;
    bool isInitialized;

    // Internal methods for each protocol
    bool playBluetooth(const std::string &mediaPath);
    bool playWiFi(const std::string &mediaPath);
    bool playAUX(const std::string &mediaPath);
};

#endif // MEDIAPLAYER_H
