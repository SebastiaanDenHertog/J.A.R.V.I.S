#include <iostream>
#include <thread>
#include <unordered_set>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <boost/make_shared.hpp>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/optional_debug_tools.h>
#include "BluetoothComm.h"
#include "NetworkManager.h"
#include "ModelRunner.h"
#include "authorization_api.h"
#include "web_service.h"
#include "InputHandler.h"
#include "TaskProcessor.h"
#include "HomeAssistantAPI.h"

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

int network_port = 15880;
unsigned short web_server_port = 15881;
int threads = 10;

bool setbluetooth = false;
std::unique_ptr<BluetoothComm> bluetoothComm;
std::thread bluetoothThread;
std::thread networkThread;
std::thread terminalInputThread;
std::unique_ptr<HomeAssistantAPI> homeAssistantAPI;
std::thread homeAssistantThread;

bool checkBluetoothAvailability()
{
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0)
    {
        return false;
    }

    int sock = hci_open_dev(dev_id);
    if (sock < 0)
    {
        return false;
    }

    hci_close_dev(sock);
    return true;
}

void terminalInputFunction(ModelRunner &modelRunner, HomeAssistantAPI &homeAssistantAPI, InputHandler &inputHandler)
{
    while (true)
    {
        std::string user_input;
        std::cout << "Enter command (type 'exit' to quit): ";
        std::getline(std::cin, user_input);

        if (user_input == "exit")
        {
            break;
        }

        if (user_input.rfind("homeassistant", 0) == 0)
        {
            std::istringstream iss(user_input);
            std::string command, entityId, service, newState;
            iss >> command >> entityId >> service >> newState;
            Task homeAssistantTask("Home Assistant Command", entityId, service, newState);
            inputHandler.addTask(homeAssistantTask);
        }
        else
        {
            Task predicted_task = modelRunner.predictTaskFromInput(user_input);
            inputHandler.addTask(predicted_task);
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef DEBUG_MODE
    std::cout << "Debug mode is ON" << std::endl;
#endif

    bool use_terminal_input = false;
    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (std::string(argv[i]) == "--terminal-input")
            {
                use_terminal_input = true;
            }
        }
    }

    std::unordered_set<std::string> allowed_keys;
    allowed_keys.insert("SampleKey");
    boost::shared_ptr<authorization_api> auth = boost::make_shared<authorization_api>(allowed_keys);
    boost::shared_ptr<web_service_context> ctx = boost::make_shared<web_service_context>(threads, auth);

    std::make_shared<web_service>(
        ctx,
        "192.168.30.11",
        web_server_port,
        "web_service")
        ->run();

    if (!checkBluetoothAvailability())
    {
        std::cerr << "Bluetooth is not available on this device." << std::endl;
    }
    else
    {
        DEBUG_PRINT("Bluetooth is available.");
        setbluetooth = true;
    }

    if (setbluetooth)
    {
        bluetoothComm = std::make_unique<BluetoothComm>();
        if (!bluetoothComm->initialize())
        {
            std::cerr << "Failed to initialize Bluetooth communication." << std::endl;
            return -1;
        }
        DEBUG_PRINT("Bluetooth communication initialized.");

        bluetoothThread = std::thread(&BluetoothComm::handleIncomingConnectionsThread, bluetoothComm.get());
        DEBUG_PRINT("Bluetooth thread started.");
    }

    // Initialize the model runner
    std::string model_path = "./models/nlp_model.tflite";
    ModelRunner modelRunner(model_path);
    modelRunner.LoadClassNames("./classes/class_nlp.xml");

    if (!modelRunner.IsLoaded())
    {
        std::cerr << "Failed to load the model." << std::endl;
    }

    NetworkManager *server = nullptr;
    try
    {
        DEBUG_PRINT("Starting NetworkManager as server.");
        server = new NetworkManager(network_port, nullptr, &modelRunner);
        networkThread = std::thread(&NetworkManager::runServer, server);
        DEBUG_PRINT("NetworkManager running.");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        delete server;
        return -1;
    }
    try
    {
        homeAssistantAPI = std::make_unique<HomeAssistantAPI>("192.168.20.10", 8123, server);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;

        return -1;
    }

    // Task handling
    InputHandler inputHandler;
    TaskProcessor taskProcessor(modelRunner, *homeAssistantAPI);

    if (use_terminal_input)
    {
        terminalInputThread = std::thread(terminalInputFunction, std::ref(modelRunner), std::ref(*homeAssistantAPI), std::ref(inputHandler));
    }

    // Process tasks
    std::thread taskProcessingThread([&]()
                                     {
        while (true)
        {
            while (inputHandler.hasTasks())
            {
                Task task = inputHandler.getNextTask();
                std::cout << "Processing task: " << task.description << std::endl;
                taskProcessor.processTask(task);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> ioThreads;
    ioThreads.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        ioThreads.emplace_back(
            [&ctx]
            {
                ctx->get_ioc()->run();
            });
    ctx->get_ioc()->run();

    // Wait for all threads to finish
    for (auto &t : ioThreads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    if (bluetoothComm && bluetoothThread.joinable())
    {
        bluetoothThread.join();
    }

    if (networkThread.joinable())
    {
        networkThread.join();
    }

    if (use_terminal_input && terminalInputThread.joinable())
    {
        terminalInputThread.join();
    }

    if (taskProcessingThread.joinable())
    {
        taskProcessingThread.join();
    }

    if (homeAssistantThread.joinable())
    {
        homeAssistantThread.join();
    }

    delete server;
    std::cout << "Application finished." << std::endl;
    return 0;
}
