/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    24-05-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the MediaPlayer class
 */

#include "MediaPlayer.h"

/**
 * @brief Constructor for MediaPlayer.
 */

MediaPlayer::MediaPlayer() : isInitialized(false) {}


/**
 * @brief Destructor for MediaPlayer.
 */
MediaPlayer::~MediaPlayer()
{
}

/**
 * @brief Initialize the media player with Bluetooth protocol.
 * @return True if successful, false otherwise.
 */

bool MediaPlayer::initializeBluetooth()
{
    // Initialize Bluetooth resources
    std::cout << "Initializing Bluetooth..." << std::endl;
    // Placeholder: Add actual initialization code
    isInitialized = true;
    return true;
}

/**
 * @brief Initialize the media player with WiFi protocol.
 * @return True if successful, false otherwise.
 */

bool MediaPlayer::initializeWiFi()
{
    // Initialize WiFi resources
    std::cout << "Initializing WiFi..." << std::endl;
    // Placeholder: Add actual initialization code
    isInitialized = true;
    return true;
}

/**
 * @brief Initialize the media player with AUX protocol.
 * @return True if successful, false otherwise.
 */

bool MediaPlayer::initializeAUX()
{
    // Initialize AUX resources
    std::cout << "Initializing AUX..." << std::endl;
    // Placeholder: Add actual initialization code
    isInitialized = true;
    return true;
}

/**
 * @brief Method to select the playback protocol.
 * @param protocol The protocol to use.
 * @return True if successful, false otherwise.
 */

bool MediaPlayer::setProtocol(const std::string &protocol)
{
    if (protocol == "Bluetooth" || protocol == "WiFi" || protocol == "AUX")
    {
        currentProtocol = protocol;
        std::cout << "Protocol set to " << protocol << std::endl;
        return true;
    }
    else
    {
        std::cerr << "Unknown protocol: " << protocol << std::endl;
        return false;
    }
}

/**
 * @brief Play media from the given path.
 * @param mediaPath The path to the media file.
 * @return True if successful, false otherwise.
 */

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

/**
 * @brief Pause the media playback.
 * @return True if successful, false otherwise.
 */

bool MediaPlayer::pause()
{
    std::cout << "Pausing media..." << std::endl;
    // Placeholder: Add actual pause code
    return true;
}

/**
 * @brief Stop the media playback.
 * @return True if successful, false otherwise.
 */

bool MediaPlayer::stop()
{
    std::cout << "Stopping media..." << std::endl;
    // Placeholder: Add actual stop code
    return true;
}

/**
 * @brief Play media over Bluetooth.
 * @param mediaPath The path to the media file.
 * @return True if successful, false otherwise.
 */

bool MediaPlayer::playBluetooth(const std::string &mediaPath)
{
    std::cout << "Playing media over Bluetooth: " << mediaPath << std::endl;
    // Placeholder: Add actual Bluetooth play code
    return true;
}

/**
 * @brief Play media over WiFi.
 * @param mediaPath The path to the media file.
 * @return True if successful, false otherwise.
 */

bool MediaPlayer::playWiFi(const std::string &mediaPath)
{
    std::cout << "Playing media over WiFi: " << mediaPath << std::endl;
    // Placeholder: Add actual WiFi play code
    return true;
}

/**
 * @brief Play media over AUX.
 * @param mediaPath The path to the media file.
 * @return True if successful, false otherwise.
 */

bool MediaPlayer::playAUX(const std::string &mediaPath)
{
    std::cout << "Playing media over AUX: " << mediaPath << std::endl;
    // Placeholder: Add actual AUX play code
    return true;
}

/**
 * @brief Set the output device for the media player.
 * @param device The device to set the output for.
 * @param output The output to set.
 * @return True if successful, false otherwise.
 */

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

/**
 * @brief Find the song based on the given entities.
 * @param entities The entities to search for.
 * @return The song file name.
 */

std::string MediaPlayer::FindSong(std::vector<std::vector<std::string>> entities)
{
    // Placeholder: Add logic to find the song based on entities
    return "Song.mp3";
}
