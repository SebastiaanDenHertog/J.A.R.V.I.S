// Watchdog.cpp
#include "Watchdog.h"
#include "webServer.h"
#include "BluetoothComm.h"
#include "AirPlayServer.h"
#include "HomeAssistantAPI.h"

Watchdog::Watchdog(Mode mode) 
    : mode(mode), running(false), webServerRunning(false), bluetoothRunning(false), airPlayRunning(false), homeAssistantRunning(false) {}

Watchdog::~Watchdog() {
    stopMonitoring();
}

void Watchdog::startMonitoring() {
    running = true;
    monitoringThread = std::thread(&Watchdog::checkServices, this);
    logEvent("Watchdog started.");
}

void Watchdog::stopMonitoring() {
    running = false;
    if (monitoringThread.joinable()) {
        monitoringThread.join();
    }
    logEvent("Watchdog stopped.");
}

void Watchdog::checkServices() {
    auto& config = ConfigurationManager::getInstance();
    while (running) {
        if (mode == Mode::SERVER) {
            // Monitor Web Server
            if (config.getUseWebServer() && !webServerRunning) {
                logEvent("Web server is not running. Attempting to start.");
                startService("webServer");
            }

            // Monitor Home Assistant
            if (config.getUseHomeAssistant() && !homeAssistantRunning) {
                logEvent("Home Assistant API is not running. Attempting to start.");
                startService("homeAssistant");
            }
        } else if (mode == Mode::CLIENT) {
            // Monitor Bluetooth
            if (config.getUseBluetooth() && !bluetoothRunning) {
                logEvent("Bluetooth is not running. Attempting to start.");
                startService("bluetooth");
            }

            // Monitor AirPlay
            if (config.getUseAirPlay() && !airPlayRunning) {
                logEvent("AirPlay is not running. Attempting to start.");
                startService("airplay");
            }
        }

        // Sleep for a while before the next check
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void Watchdog::startService(const std::string &service) {
    if (service == "webServer") {
        webServerRunning = true;
        std::thread([&]() {
            setup_server(
                ConfigurationManager::getInstance().getWebServerPort()
            );
            webServerRunning = false;
        }).detach();
        logEvent("Web server started.");
    } else if (service == "bluetooth") {
        bluetoothRunning = true;
        std::thread([&]() {
            BluetoothComm bluetoothComm;
            if (bluetoothComm.initialize()) {
                bluetoothComm.handleIncomingConnections();
            } else {
                logEvent("Bluetooth failed to start.");
            }
            bluetoothRunning = false;
        }).detach();
        logEvent("Bluetooth service started.");
    } else if (service == "airplay") {
        airPlayRunning = true;
        std::thread([&]() {
            AirPlayServer airplayServer;
            airplayServer.run();
            airplayServer.main_loop();
            airPlayRunning = false;
        }).detach();
        logEvent("AirPlay server started.");
    } else if (service == "homeAssistant") {
        homeAssistantRunning = true;
        std::thread([&]() {
            try {
                auto& config = ConfigurationManager::getInstance();
                HomeAssistantAPI homeAssistantAPI(config.getHomeAssistantIP(), config.getHomeAssistantPort(), config.getHomeAssistantToken());
                homeAssistantAPI.start();
                homeAssistantRunning = false;
            } catch (const std::exception &e) {
                logEvent("Failed to start Home Assistant: " + std::string(e.what()));
                homeAssistantRunning = false;
            }
        }).detach();
        logEvent("Home Assistant API started.");
    }
}

void Watchdog::logEvent(const std::string &message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::cerr << "[Watchdog] " << message << std::endl;
}
