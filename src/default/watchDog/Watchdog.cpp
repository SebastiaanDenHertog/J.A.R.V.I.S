#include "Watchdog.h"

extern NetworkManager *serverNetworkManager;

#ifdef SERVER_BUILD
extern std::unique_ptr<ModelRunner> nerModel;
extern std::unique_ptr<ModelRunner> classificationModel;
extern std::unique_ptr<TaskProcessor> taskProcessor;
extern std::unique_ptr<InputHandler> inputHandler;
#endif

Watchdog::Watchdog(Mode mode)
    : mode(mode), running(false), webServerRunning(false), bluetoothRunning(false), airPlayRunning(false), homeAssistantRunning(false) {}

Watchdog::~Watchdog()
{
    stopMonitoring();
    if (bluetoothComm)
    {
        bluetoothComm->terminate();
        bluetoothComm.reset();
    }
}

void Watchdog::startMonitoring()
{
    running = true;
    monitoringThread = std::thread(&Watchdog::checkServices, this);
    logEvent("Watchdog started.");
}

void Watchdog::stopMonitoring()
{
    running = false;
    if (monitoringThread.joinable())
    {
        monitoringThread.join();
    }
    logEvent("Watchdog stopped.");
}

void Watchdog::checkServices()
{
    ConfigurationManager &configManager = ConfigurationManager::getInstance();
    while (running)
    {
        Configuration config = configManager.getConfiguration();

        if (mode == Mode::SERVER)
        {
            // Monitor Web Server
            logEvent("Checking services...");
            logEvent("Web server: " + std::to_string(config.use_web_server));
            if (config.use_web_server && !webServerRunning)
            {
                logEvent("Web server is not running. Attempting to start.");
                startService("webServer");
            }

            // Monitor Home Assistant
            if (config.use_home_assistant && !homeAssistantRunning)
            { // Assuming use_server controls Home Assistant
                logEvent("Home Assistant API is not running. Attempting to start.");
                startService("homeAssistant");
            }
        }
        else if (mode == Mode::CLIENT)
        {
            // Monitor Bluetooth
            if (config.use_bluetooth && !bluetoothRunning)
            {
                logEvent("Bluetooth is not running. Attempting to start.");
                startService("bluetooth");
            }

            // Monitor AirPlay
            if (config.use_airplay && !airPlayRunning)
            {
                logEvent("AirPlay is not running. Attempting to start.");
                startService("airplay");
            }
        }

        // Sleep for a while before the next check
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void Watchdog::startService(const std::string &service)
{
    // Get the current configuration manager instance
    ConfigurationManager &configManager = ConfigurationManager::getInstance();
    Configuration config = configManager.getConfiguration();

    if (service == "webServer")
    {
        webServerRunning = true;
        std::thread([&, config]()
                    {
            setup_server(
                config.web_server_secure,
                config.web_server_cert_path,
                config.web_server_key_path,
                config.web_server_port,
                config.threads
            );
            webServerRunning = false; })
            .detach();
        logEvent("Web server started.");
    }
    else if (service == "bluetooth")
    {
        bluetoothRunning = true;

        // Initialize the BluetoothComm instance and start the handling thread
        bluetoothComm = std::make_unique<BluetoothComm>();

        std::thread([this]()
                    {
        if (bluetoothComm->initialize()) {
            logEvent("Bluetooth communication initialized.");
            bluetoothComm->handleIncomingConnectionsThread();
        } else {
            logEvent("Bluetooth failed to initialize.");
            bluetoothRunning = false;
            return;
        }
        bluetoothRunning = false; })
            .detach();

        logEvent("Bluetooth service started.");
    }

#ifdef CLIENT_BUILD
    else if (service == "airplay")
    {
        airPlayRunning = true;
        std::thread([&]()
                    {
            AirPlayServer airplayServer;
            airplayServer.run();
            airplayServer.main_loop();
            airPlayRunning = false; })
            .detach();
        logEvent("AirPlay server started.");
    }
#endif
    else if (service == "homeAssistant")
    {
        homeAssistantRunning = true;
        std::thread([&, config]()
                    {
            try {
                if (!serverNetworkManager) {
                    logEvent("Home Assistant requires a running server NetworkManager.");
                    homeAssistantRunning = false;
                    return;
                }

                HomeAssistantAPI homeAssistantAPI(
                    config.home_assistant_ip,
                    config.home_assistant_port,
                    config.home_assistant_token,
                    serverNetworkManager  // Pass the running server NetworkManager instance
                );
                homeAssistantRunning = false;
            } catch (const std::exception &e) {
                logEvent("Failed to start Home Assistant: " + std::string(e.what()));
                homeAssistantRunning = false;
            } })
            .detach();
        logEvent("Home Assistant API started.");
    }
}

void Watchdog::logEvent(const std::string &message)
{
    std::lock_guard<std::mutex> lock(logMutex);
    std::cerr << "[Watchdog] " << message << std::endl;
}
