#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

#include <string>
#include <mutex>

class ConfigurationManager
{
public:
    static ConfigurationManager &getInstance();

    // Getters and setters for various configuration settings
    bool getUseWebServer() const;
    void setUseWebServer(bool value);

    unsigned short getWebServerPort() const;
    void setWebServerPort(unsigned short port);

    std::string getMainServerIP() const;
    void setMainServerIP(const std::string &ip);

    int getMainServerPort() const;
    void setMainServerPort(int port);

    bool getUseBluetooth() const;
    void setUseBluetooth(bool value);

    bool getUseAirPlay() const;
    void setUseAirPlay(bool value);

private:
    ConfigurationManager(); // Private constructor for singleton
    mutable std::mutex configMutex;

    bool useWebServer = true;
    unsigned short webServerPort = 15881;
    std::string mainServerIP = "127.0.0.1";
    int mainServerPort = 15880;
    bool useBluetooth = false;
    bool useAirPlay = false;
};

#endif // CONFIGURATIONMANAGER_H
