/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    24-05-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the MediaPlayer class
 **/

#include "MediaPlayer.h"

MediaPlayer::MediaPlayer() : isInitialized(false) {}

MediaPlayer::~MediaPlayer()
{
    // Cleanup resources if necessary
}

bool MediaPlayer::initializeBluetooth()
{
    // Initialize Bluetooth resources
    std::cout << "Initializing Bluetooth..." << std::endl;
    // Placeholder: Add actual initialization code
    isInitialized = true;
    return true;
}

bool MediaPlayer::initializeWiFi()
{
    // Initialize WiFi resources
    std::cout << "Initializing WiFi..." << std::endl;
    // Placeholder: Add actual initialization code
    isInitialized = true;
    return true;
}

bool MediaPlayer::initializeAUX()
{
    // Initialize AUX resources
    std::cout << "Initializing AUX..." << std::endl;
    // Placeholder: Add actual initialization code
    isInitialized = true;
    return true;
}

void MediaPlayer::setProtocol(const std::string &protocol)
{
    if (protocol == "Bluetooth" || protocol == "WiFi" || protocol == "AUX")
    {
        currentProtocol = protocol;
        std::cout << "Protocol set to " << protocol << std::endl;
    }
    else
    {
        std::cerr << "Unknown protocol: " << protocol << std::endl;
    }
}

bool MediaPlayer::play(const std::string &mediaPath)
{
    if (!isInitialized)
    {
        std::cerr << "Media player is not initialized." << std::endl;
        return false;
    }

    if (currentProtocol == "Bluetooth")
    {
        return playBluetooth(mediaPath);
    }
    else if (currentProtocol == "WiFi")
    {
        return playWiFi(mediaPath);
    }
    else if (currentProtocol == "AUX")
    {
        return playAUX(mediaPath);
    }
    else
    {
        std::cerr << "No protocol selected." << std::endl;
        return false;
    }
}

bool MediaPlayer::pause()
{
    std::cout << "Pausing media..." << std::endl;
    // Placeholder: Add actual pause code
    return true;
}

bool MediaPlayer::stop()
{
    std::cout << "Stopping media..." << std::endl;
    // Placeholder: Add actual stop code
    return true;
}

bool MediaPlayer::playBluetooth(const std::string &mediaPath)
{
    std::cout << "Playing media over Bluetooth: " << mediaPath << std::endl;
    // Placeholder: Add actual Bluetooth play code
    return true;
}

bool MediaPlayer::playWiFi(const std::string &mediaPath)
{
    std::cout << "Playing media over WiFi: " << mediaPath << std::endl;
    // Placeholder: Add actual WiFi play code
    return true;
}

bool MediaPlayer::playAUX(const std::string &mediaPath)
{
    std::cout << "Playing media over AUX: " << mediaPath << std::endl;
    // Placeholder: Add actual AUX play code
    return true;
}

bool MediaPlayer::setoutput(ClientInfo device, std::string output)
{
    for (auto musicOutput : device.getMusicOutputs())
    {
        if (musicOutput == output)
        {
            std::cout << "Setting output to: " << output << std::endl;
            setProtocol(output);
            return true;
        }
    }

    std::cerr << "Output not found: " << output << std::endl;
    return false;
}

std::string MediaPlayer::FindSong(std::vector<std::vector<std::string>> entities)
{
    // Placeholder: Add logic to find the song based on entities
    return "Song.mp3";
}
