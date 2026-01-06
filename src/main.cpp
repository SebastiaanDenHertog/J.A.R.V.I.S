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
#include <fstream>

#include "BluetoothComm.h"
#include "NetworkManager.h"
#include "webServer.h"
#include "ClientInfo.h"
#include "Configuration.h"
#include "Watchdog.h"

#ifdef CLIENT_BUILD
#include "PixelRing.h"
#include "ReSpeaker.h"
#include "HardwareInterface.h"
#include "AirPlayServer.h"
#endif

#ifdef SERVER_BUILD
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
#include "IntentRouter.h"
#include "counter.h"
#include "registry.h"
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#ifdef SERVER_BUILD
std::unique_ptr<ModelRunner> nerModel;
std::unique_ptr<ModelRunner> classificationModel;
std::unique_ptr<TaskProcessor> taskProcessor;
std::unique_ptr<InputHandler> inputHandler;
std::unique_ptr<IntentRouter> intentRouter;
NetworkManager *serverNetworkManager = nullptr;
const std::string input_op  = "serving_default_input_ids:0";
const std::string output_op = "StatefulPartitionedCall:0";
#else
std::unique_ptr<NetworkManager> clientNetworkManager;
#endif

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

std::atomic<bool> global_running(true);

/**
 * @brief Signal handler to gracefully shutdown the application.
 */
void signal_handler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        std::cout << "\nReceived shutdown signal. Exiting..." << std::endl;
        global_running = false;
    }
}

#ifdef CLIENT_BUILD
/**
 * @brief Send example speech data to the server.
 * @param client Reference to the NetworkManager client.
 */
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
#endif

#ifdef SERVER_BUILD

/**
 * @brief Function to handle terminal input for commands.
 * @param inputHandlerObj Reference to the InputHandler object.
 * @param taskProcessorObj Reference to the TaskProcessor object.
 */
void terminalInputFunction(InputHandler &inputHandlerObj, TaskProcessor &taskProcessorObj, NetworkManager &networkManager)
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
        std::vector<std::pair<std::string, std::string>> emptyVectorPair;
        std::vector<std::string> emptyVector;
        UserCommand user_command(user_input,emptyVectorPair, nullptr, emptyVector);
        Task task(taskProcessorObj.createTaskNumber(), user_input, 1, {"server_terminal", networkManager.getIpAddress(), 15880, {}}, Task::Ner, user_command);
        inputHandlerObj.addTask(task);
        taskProcessorObj.processTask(task);
    }
}

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
void start_server_network_manager(int server_port)
{
    try
    {
        serverNetworkManager = new NetworkManager(server_port, NetworkManager::Protocol::TCP, nerModel.get(), classificationModel.get());
        std::thread networkThread(&NetworkManager::runServer, serverNetworkManager);
        networkThread.detach();
        DEBUG_PRINT("Server NetworkManager started on port " << server_port);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to start Server NetworkManager: " << e.what() << std::endl;
    }
}

/**
 * @brief Stop the server NetworkManager.
 */
void stop_server_network_manager()
{
    if (serverNetworkManager)
    {
        delete serverNetworkManager;
        serverNetworkManager = nullptr;
        DEBUG_PRINT("Server NetworkManager stopped.");
    }
}

/**
 * @brief Initialize the models and TaskProcessor.
 */
void initialize_models_and_task_processor()
{
    try
    {
        Configuration config = ConfigurationManager::getInstance().getConfiguration();
        nerModel = std::make_unique<ModelRunner>(config.Root +"models/ner_model",input_op, output_op);
        classificationModel = std::make_unique<ModelRunner>(config.Root +"models/classification_model",input_op, output_op);
        // Create the IntentRouter and store it in the dedicated pointer
        intentRouter = std::make_unique<IntentRouter>(config.Root +"config/intents.json");

        nerModel->LoadTokenizer(config.Root +"models/ner_tokenizer.json");
        nerModel->LoadLabels(config.Root +"models/ner_labels.json");

        classificationModel->LoadTokenizer(config.Root +"models/classification_tokenizer.json");
        classificationModel->LoadLabels(config.Root +"models/classification_type_labels.json");

        inputHandler = std::make_unique<InputHandler>();
        taskProcessor = std::make_unique<TaskProcessor>(nullptr, *nerModel, *classificationModel, *intentRouter, *inputHandler, *taskProcessor);
        DEBUG_PRINT("Models and TaskProcessor initialized.");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to initialize models or TaskProcessor: " << e.what() << std::endl;
        exit(EXIT_FAILURE); 
    }
}

void start_terminal_input()
{
    if (!nerModel || !classificationModel || !taskProcessor || !inputHandler || !intentRouter)
    {
        std::cerr << "Models or TaskProcessor are not properly initialized." << std::endl;
        return;
    }

    std::thread terminalThread([&]()
                               { terminalInputFunction(*inputHandler, *taskProcessor,*serverNetworkManager); });

    terminalThread.detach();
    DEBUG_PRINT("Terminal input thread started.");
}
#endif

/**
 * @brief Check if a file exists.
 * @param filename The path to the file.
 * @return true if the file exists, false otherwise.
 */
bool fileExists(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Configuration file not found: " << filename << ". Using default settings." << std::endl;
        return false;
    }
    return true;
}

void createDefaultConfig(const std::string &filename)
{
    Configuration default_config;

    default_config.use_server = false;
    default_config.main_server_port = 15880;
    default_config.use_bluetooth = false;

    ConfigurationManager &configManager = ConfigurationManager::getInstance();

    configManager.updateConfiguration(default_config);

    configManager.saveConfiguration(filename);

    std::cout << "Default configuration created at " << filename << std::endl;
}

int main(int /*argc*/, char * /*argv*/[])
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    ConfigurationManager &configManager = ConfigurationManager::getInstance();
    std::string configFilePath = configManager.getConfiguration().configFilePath;

    if (!fileExists(configFilePath))
    {
        createDefaultConfig(configFilePath);
    }

    configManager.loadConfiguration(configFilePath);
    Configuration initial_config = configManager.getConfiguration();

    Mode currentMode = (initial_config.get_mode_string() == "SERVER") ? Mode::SERVER : Mode::CLIENT;

    Watchdog watchdog(currentMode);

#ifdef SERVER_BUILD
    if (initial_config.use_server)
    {
        initialize_models_and_task_processor();
        start_server_network_manager(initial_config.main_server_port); // Use the configured server port
        start_terminal_input();                                        // Start terminal input for processing commands
    }
#endif

    watchdog.startMonitoring();

    while (global_running)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    watchdog.stopMonitoring();
#ifdef SERVER_BUILD
    stop_server_network_manager();
#endif

    std::cout << "Application exited gracefully." << std::endl;
    return 0;
}
