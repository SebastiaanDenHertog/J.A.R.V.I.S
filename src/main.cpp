/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    03-04-2024
 * @Date updated    09-10-2024 (By: Sebastiaan den Hertog)
 * @Description     Main application that handles both client and server functionalities with web-based configuration.
 **/

#include <iostream>
#include <thread>
#include <memory>
#include <string>
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <atomic>
#include <csignal>
#include <chrono>
#include <unordered_map>

// Common headers
#include "BluetoothComm.h"
#include "NetworkManager.h"
#include "webServer.h"
#include "ClientInfo.h"
#include "Configuration.h"
#include "Watchdog.h"

#ifdef CLIENT_BUILD
// Client-specific headers
#include "PixelRing.h"
#include "ReSpeaker.h"
#include "HardwareInterface.h"
#include "AirPlayServer.h"
#endif

#ifdef SERVER_BUILD
// Server-specific headers
#include <unordered_set>
#include <cstdlib>
#include <boost/make_shared.hpp>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/optional_debug_tools.h>
#include "ModelRunner.h"
#include "InputHandler.h"
#include "TaskProcessor.h"
#include "HomeAssistantAPI.h"
#include "counter.h"
#include "registry.h"
#endif

// For JSON parsing
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#ifdef SERVER_BUILD
std::unique_ptr<ModelRunner> nerModel;
std::unique_ptr<ModelRunner> classificationModel;
std::unique_ptr<TaskProcessor> taskProcessor;
std::unique_ptr<InputHandler> inputHandler;
#endif

// Debug print macro
#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

// Global running flag
std::atomic<bool> global_running(true);

// Signal handler to gracefully shutdown
void signal_handler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        std::cout << "\nReceived shutdown signal. Exiting..." << std::endl;
        global_running = false;
    }
}

// Function to get the local IP address of the server
const char *getLocalIP()
{
    std::string local_ip;
    std::string cmd = "hostname -I";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        std::cerr << "popen() failed!" << std::endl;
        return "";
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
    {
        local_ip += buffer;
    }
    local_ip.erase(std::remove(local_ip.begin(), local_ip.end(), '\n'), local_ip.end());
    std::stringstream ss(local_ip);
    std::string first_ip;
    ss >> first_ip;
    return strdup(first_ip.c_str());
}

// Function to check if Bluetooth is available
bool checkBluetoothAvailability()
{
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0)
        return false;
    int sock = hci_open_dev(dev_id);
    if (sock < 0)
        return false;
    hci_close_dev(sock);
    return true;
}

#ifdef CLIENT_BUILD
// Send speech data to the server
void send_speech_data(NetworkManager &client)
{
    try
    {
        const char *soundData = "example sound data";
        client.sendSoundData(reinterpret_cast<const uint8_t *>(soundData), strlen(soundData));
        client.receiveResponse();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error sending speech data: " << e.what() << std::endl;
    }
}

// Convert string to Task::TaskType
Task::TaskType stringToTaskType(const std::string &str)
{
    static const std::unordered_map<std::string, Task::TaskType> strToTaskType = {
        {"Book", Task::Book},
        {"Calculate", Task::Calculate},
        {"Calendar", Task::Calendar},
        {"Call", Task::Call},
        {"Connect", Task::Connect},
        {"ControlHeating", Task::ControlHeating},
        {"ControlLight", Task::ControlLight},
        {"Define", Task::Define},
        {"Email", Task::Email},
        {"Find", Task::Find},
        {"GetRecipe", Task::GetRecipe},
        {"GetShippingInfo", Task::GetShippingInfo},
        {"Locate", Task::Locate},
        {"Message", Task::Message},
        {"Navigate", Task::Navigate},
        {"NewsQuery", Task::NewsQuery},
        {"OrderItem", Task::OrderItem},
        {"PauseMusic", Task::PauseMusic},
        {"PauseVideo", Task::PauseVideo},
        {"PlayMusic", Task::PlayMusic},
        {"PlayVideo", Task::PlayVideo},
        {"Read", Task::Read},
        {"Recommend", Task::Recommend},
        {"ResumeVideo", Task::ResumeVideo},
        {"SetAlarm", Task::SetAlarm},
        {"SetTimer", Task::SetTimer},
        {"SetVolume", Task::SetVolume},
        {"ShoppingList", Task::ShoppingList},
        {"Summarize", Task::Summarize},
        {"Translate", Task::Translate},
        {"WeatherQuery", Task::WeatherQuery}};
    auto it = strToTaskType.find(str);
    return it != strToTaskType.end() ? it->second : Task::ERROR;
}

// Function for handling terminal input and processing tasks (server-side)
void terminalInputFunction(ModelRunner &nerModel, ModelRunner &classificationModel, HomeAssistantAPI *homeAssistantAPI, InputHandler &inputHandler, TaskProcessor &taskProcessor)
{
    while (global_running)
    {
        std::string user_input;
        std::cout << "Enter command (type 'exit' to quit): ";
        std::getline(std::cin, user_input);
        if (user_input == "exit")
        {
            global_running = false;
            break;
        }
        auto [_, predicted_entities] = nerModel.PredictlabelFromInput(user_input); // Ignore the first part
        std::vector<std::pair<std::string, std::string>> sentence_entities;
        std::istringstream iss(user_input);
        std::string word;
        int entity_index = 0;
        while (iss >> word && entity_index < predicted_entities.size())
        {
            sentence_entities.push_back({word, predicted_entities[entity_index]});
            entity_index++;
        }
        std::cout << "Sentence and Entities: " << std::endl;
        for (const auto &pair : sentence_entities)
        {
            std::cout << "Word: " << pair.first << " -> Entity: " << pair.second << std::endl;
        }
        std::string sentence_label = classificationModel.ClassifySentence(user_input);
        std::cout << "Intent: " << sentence_label << std::endl;
        UserCommand user_command(user_input, sentence_entities, sentence_label, predicted_entities);
        Task::TaskType taskType = stringToTaskType(sentence_label);
        Task task(user_input, 1, {"client", getLocalIP(), 15880, {}}, taskType, user_command);
        inputHandler.addTask(task);
        taskProcessor.processTask(task);
    }
}
#endif

#ifdef SERVER_BUILD
// Placeholder for server logic
void run_server(const Configuration &config)
{
    try
    {
        if (config.use_server)
        {
            DEBUG_PRINT("Server: Initializing server components.");
        }
        while (global_running && config.use_server)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        DEBUG_PRINT("Server: Exiting server loop.");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server Exception: " << e.what() << std::endl;
    }
}
#endif

#ifdef CLIENT_BUILD
// Placeholder for client logic
void run_client(const Configuration &config)
{
    try
    {
        if (config.use_bluetooth)
        {
            BluetoothComm bluetoothComm;
            bluetoothComm.initialize();
            DEBUG_PRINT("Client: Bluetooth communication running.");
        }
        if (config.use_airplay)
        {
            AirPlayServer airplayServer;
            airplayServer.run();
            DEBUG_PRINT("Client: AirPlay server running.");
        }
        while (global_running && config.use_client)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        DEBUG_PRINT("Client: Exiting client loop.");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client Exception: " << e.what() << std::endl;
    }
}
#endif

#ifdef SERVER_BUILD
NetworkManager *serverNetworkManager = nullptr; // For server-side communication
#endif

#ifdef CLIENT_BUILD
NetworkManager *clientNetworkManager = nullptr; // For client-side communication
#endif

#ifdef SERVER_BUILD
// Function to start the server network manager
void start_server_network_manager(int server_port)
{
    try
    {
        serverNetworkManager = new NetworkManager(server_port, nullptr, NetworkManager::Protocol::TCP);
        std::thread networkThread(&NetworkManager::runServer, serverNetworkManager);
        networkThread.detach();
        DEBUG_PRINT("Server NetworkManager started on port " << server_port);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to start Server NetworkManager: " << e.what() << std::endl;
    }
}

// Function to stop the server network manager
void stop_server_network_manager()
{
    if (serverNetworkManager)
    {
        serverNetworkManager->stop(); // Assuming NetworkManager has a stop function
        delete serverNetworkManager;
        serverNetworkManager = nullptr;
        DEBUG_PRINT("Server NetworkManager stopped.");
    }
}

// Initialize ModelRunners and TaskProcessor for NLP tasks
void initialize_models_and_task_processor()
{
    try
    {
        nerModel = std::make_unique<ModelRunner>("./models/ner_model.tflite");
        classificationModel = std::make_unique<ModelRunner>("./models/classification_model.tflite");

        nerModel->LoadTokenizer("./models/ner_tokenizer.json");
        nerModel->LoadLabels("./models/ner_labels.json");

        classificationModel->LoadTokenizer("./models/classification_tokenizer.json");
        classificationModel->LoadLabels("./models/classification_type_labels.json");

        inputHandler = std::make_unique<InputHandler>();
        taskProcessor = std::make_unique<TaskProcessor>(nullptr, *nerModel, *classificationModel);

        DEBUG_PRINT("Models and TaskProcessor initialized.");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to initialize models or TaskProcessor: " << e.what() << std::endl;
        exit(EXIT_FAILURE); // Exit if models fail to load since they are critical
    }
}

#endif

#ifdef CLIENT_BUILD
// Function to start the client network manager
void start_client_network_manager(const std::string &server_ip, int server_port)
{
    try
    {
        clientNetworkManager = new NetworkManager(server_port, server_ip.c_str(), NetworkManager::Protocol::TCP);
        std::thread clientThread(&NetworkManager::connectClient, clientNetworkManager);
        clientThread.detach();
        DEBUG_PRINT("Client NetworkManager started, connecting to " << server_ip << ":" << server_port);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to start Client NetworkManager: " << e.what() << std::endl;
    }
}

// Function to stop the client network manager
void stop_client_network_manager()
{
    if (clientNetworkManager)
    {
        clientNetworkManager->disconnect(); // Assuming NetworkManager has a disconnect function
        delete clientNetworkManager;
        clientNetworkManager = nullptr;
        DEBUG_PRINT("Client NetworkManager stopped.");
    }
}
#endif
#ifdef SERVER_BUILD
// Function to start terminal input handling for the server
void start_terminal_input()
{
    if (!nerModel || !classificationModel || !taskProcessor || !inputHandler)
    {
        std::cerr << "Models or TaskProcessor are not properly initialized." << std::endl;
        return;
    }

    std::thread terminalThread([&]()
                               { terminalInputFunction(*nerModel, *classificationModel, nullptr, *inputHandler, *taskProcessor); });

    terminalThread.detach();
    DEBUG_PRINT("Terminal input thread started.");
}
#endif

int main(int argc, char *argv[])
{
    // Register signal handlers for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Initialize Configuration Manager with default or loaded config
    ConfigurationManager &configManager = ConfigurationManager::getInstance();
    configManager.loadConfiguration("config.json"); // Load from file if exists
    Configuration initial_config = configManager.getConfiguration();

    // Determine mode and initialize Watchdog
    Mode currentMode = initial_config.use_server ? Mode::SERVER : Mode::CLIENT;
    Watchdog watchdog(currentMode);

// Start the appropriate NetworkManager and models based on the mode
#ifdef SERVER_BUILD
    if (initial_config.use_server)
    {
        initialize_models_and_task_processor();
        start_server_network_manager(initial_config.main_server_port); // Use the configured server port
        start_terminal_input();                                        // Start terminal input for processing commands
    }
#endif

#ifdef CLIENT_BUILD
    if (initial_config.use_client)
    {
        start_client_network_manager(initial_config.main_server_ip, initial_config.main_server_port);
        std::thread clientThread(run_client, initial_config);
        clientThread.detach();
    }
#endif

    // Start Watchdog Monitoring
    watchdog.startMonitoring();

    // Main loop to monitor configuration changes and handle graceful shutdown
    while (global_running)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Stop Watchdog and NetworkManager instances before exiting
    watchdog.stopMonitoring();
#ifdef SERVER_BUILD
    stop_server_network_manager();
#endif
#ifdef CLIENT_BUILD
    stop_client_network_manager();
#endif

    std::cout << "Application exited gracefully." << std::endl;
    return 0;
}
