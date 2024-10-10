#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>

#include "BluetoothComm.h"
#include "webServer.h"
#include "HomeAssistantAPI.h"
#include "Configuration.h"
#include "TaskProcessor.h"      
#include "InputHandler.h"
#ifdef CLIENT_BUILD
#include "AirPlayServer.h"
#endif


// Define Mode enumeration if not defined elsewhere
enum class Mode
{
    CLIENT,
    SERVER
};

// ThreadInfo struct with move semantics
struct ThreadInfo
{
    std::string name;
    std::atomic<bool> is_running;
    std::function<void()> thread_function;
    std::unique_ptr<std::thread> thread;

    // Constructor
    ThreadInfo(std::string name, std::function<void()> func)
        : name(std::move(name)), is_running(true), thread_function(std::move(func)), thread(nullptr) {}

    // Delete copy constructor and copy assignment
    ThreadInfo(const ThreadInfo &) = delete;
    ThreadInfo &operator=(const ThreadInfo &) = delete;

    // Move constructor
    ThreadInfo(ThreadInfo &&other) noexcept
        : name(std::move(other.name)), is_running(other.is_running.load()),
          thread_function(std::move(other.thread_function)), thread(std::move(other.thread)) {}

    // Move assignment
    ThreadInfo &operator=(ThreadInfo &&other) noexcept
    {
        if (this != &other)
        {
            name = std::move(other.name);
            is_running.store(other.is_running.load());
            thread_function = std::move(other.thread_function);
            thread = std::move(other.thread);
        }
        return *this;
    }
};

class Watchdog
{
public:
    // Constructor with Mode
    Watchdog(Mode mode);

    // Destructor
    ~Watchdog();

    // Start monitoring services
    void startMonitoring();

    // Stop monitoring services
    void stopMonitoring();

private:
    Mode mode;
    std::atomic<bool> running;
    bool webServerRunning;
    bool bluetoothRunning;
    bool airPlayRunning;
    bool homeAssistantRunning;

    std::thread monitoringThread;
    std::mutex logMutex;

    std::unique_ptr<BluetoothComm> bluetoothComm;
    std::thread bluetoothThread;

    // Function to monitor services
    void checkServices();

    // Function to start a specific service
    void startService(const std::string &service);

    // Function to log events
    void logEvent(const std::string &message);
};

#endif // WATCHDOG_H
