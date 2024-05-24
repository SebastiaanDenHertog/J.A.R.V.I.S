#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <string>
#include <iostream>

class MediaPlayer {
public:
    MediaPlayer();
    virtual ~MediaPlayer();

    // Initialize the media player with different protocols
    bool initializeBluetooth();
    bool initializeWiFi();
    bool initializeAUX();

    // Media control methods
    bool play(const std::string &mediaPath);
    bool pause();
    bool stop();

    // Method to select the playback protocol
    void setProtocol(const std::string &protocol);

private:
    std::string currentProtocol;
    bool isInitialized;

    // Internal methods for each protocol
    bool playBluetooth(const std::string &mediaPath);
    bool playWiFi(const std::string &mediaPath);
    bool playAUX(const std::string &mediaPath);
};

#endif // MEDIAPLAYER_H
