
#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "ConfigurationManager.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

enum class Mode {
    CLIENT,
    SERVER
};

class Watchdog {
public:
    Watchdog(Mode mode);
    ~Watchdog();

    void startMonitoring();
    void stopMonitoring();

private:
    void checkServices();
    void startService(const std::string &service);
    void stopService(const std::string &service);
    void restartService(const std::string &service);
    void logEvent(const std::string &message);

    Mode mode;
    std::atomic<bool> running;
    std::thread monitoringThread;
    std::mutex logMutex;

    std::atomic<bool> webServerRunning;
    std::atomic<bool> bluetoothRunning;
    std::atomic<bool> airPlayRunning;
    std::atomic<bool> homeAssistantRunning;

    void monitorWebServer();
    void monitorBluetooth();
    void monitorAirPlay();
    void monitorHomeAssistant();
};

#endif // WATCHDOG_H
