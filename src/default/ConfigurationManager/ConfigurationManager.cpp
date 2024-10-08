// ConfigurationManager.cpp
#include "ConfigurationManager.h"

ConfigurationManager& ConfigurationManager::getInstance() {
    static ConfigurationManager instance;
    return instance;
}

bool ConfigurationManager::getUseWebServer() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return useWebServer;
}

void ConfigurationManager::setUseWebServer(bool value) {
    std::lock_guard<std::mutex> lock(configMutex);
    useWebServer = value;
}

unsigned short ConfigurationManager::getWebServerPort() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return webServerPort;
}

void ConfigurationManager::setWebServerPort(unsigned short port) {
    std::lock_guard<std::mutex> lock(configMutex);
    webServerPort = port;
}

std::string ConfigurationManager::getMainServerIP() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return mainServerIP;
}

void ConfigurationManager::setMainServerIP(const std::string& ip) {
    std::lock_guard<std::mutex> lock(configMutex);
    mainServerIP = ip;
}

int ConfigurationManager::getMainServerPort() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return mainServerPort;
}

void ConfigurationManager::setMainServerPort(int port) {
    std::lock_guard<std::mutex> lock(configMutex);
    mainServerPort = port;
}

bool ConfigurationManager::getUseBluetooth() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return useBluetooth;
}

void ConfigurationManager::setUseBluetooth(bool value) {
    std::lock_guard<std::mutex> lock(configMutex);
    useBluetooth = value;
}

bool ConfigurationManager::getUseAirPlay() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return useAirPlay;
}

void ConfigurationManager::setUseAirPlay(bool value) {
    std::lock_guard<std::mutex> lock(configMutex);
    useAirPlay = value;
}
