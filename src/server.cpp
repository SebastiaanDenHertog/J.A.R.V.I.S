#include <iostream>
#include <thread>
#include <unordered_set>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <utility>
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
#include "ClientInfo.h"

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

int network_port = 15880;
unsigned short web_server_port = 15881;
int threads = 10;

bool setbluetooth = false;
bool use_homeassistant = false;
std::string homeassistant_ip;
int homeassistant_port = 0;
std::string homeassistant_token;
std::unique_ptr<BluetoothComm> bluetoothComm;
std::thread bluetoothThread;
std::thread networkThread;
std::thread terminalInputThread;
std::unique_ptr<HomeAssistantAPI> homeAssistantAPI;
std::thread homeAssistantThread;

std::string getLocalIP()
{
    std::string local_ip;
    std::string cmd = "hostname -I";
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "popen() failed!" << std::endl;
        return "";
    }
    char buffer[128];
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            local_ip += buffer;
        }
    }
    pclose(pipe);
    local_ip.erase(std::remove(local_ip.begin(), local_ip.end(), '\n'), local_ip.end());

    // Split the string by spaces and return the first IP address
    std::istringstream iss(local_ip);
    std::string first_ip;
    iss >> first_ip;

    return first_ip;
}

ClientInfo device{"server",
                  getLocalIP(),
                  network_port,
                  {}};

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

#include <unordered_map>
#include <string>

// Function to map string to TaskType enum
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
    if (it != strToTaskType.end())
    {
        return it->second;
    }
    else
    {
        return Task::ERROR;
    }
}

void terminalInputFunction(ModelRunner &nerModel, ModelRunner &classificationModel, HomeAssistantAPI *homeAssistantAPI, InputHandler &inputHandler, TaskProcessor &taskProcessor)
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

        // Get predicted intent and entities
        auto [predicted_intent, predicted_entities] = nerModel.PredictlabelFromInput(user_input);

        // Store the sentence and the entities
        std::vector<std::pair<std::string, std::string>> sentence_entities;

        std::istringstream iss(user_input);
        std::string word;
        int entity_index = 0;

        while (iss >> word && entity_index < predicted_entities.size())
        {
            sentence_entities.push_back({word, predicted_entities[entity_index]});
            entity_index++;
        }

        // Output the sentence and its entities
        std::cout << "Sentence and Entities: " << std::endl;
        for (const auto &pair : sentence_entities)
        {
            std::cout << "Word: " << pair.first << " -> Entity: " << pair.second << std::endl;
        }

        std::string sentence_label = classificationModel.ClassifySentence(user_input);
        std::cout << "Intent: " << sentence_label << std::endl;

        // Convert predicted intent to Task::TaskType
        Task::TaskType taskType = stringToTaskType(sentence_label);

        Task task(predicted_intent, 1, device, taskType, {predicted_entities});
        inputHandler.addTask(task);
        taskProcessor.processTask(task);
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

            if (std::string(argv[i]) == "--network-port")
            {
                if (i + 1 < argc)
                {
                    network_port = std::atoi(argv[i + 1]);
                    device.setPort(network_port);
                }
            }

            if (std::string(argv[i]) == "--web-server-port")
            {
                if (i + 1 < argc)
                {
                    web_server_port = std::atoi(argv[i + 1]);
                }
            }

            if (std::string(argv[i]) == "--threads")
            {
                if (i + 1 < argc)
                {
                    threads = std::atoi(argv[i + 1]);
                }
            }
            if (std::string(argv[i]) == "--homeassistant")
            {
                use_homeassistant = true;
                if (i + 3 < argc)
                {
                    homeassistant_ip = argv[i + 1];
                    homeassistant_port = std::stoi(argv[i + 2]);
                    homeassistant_token = argv[i + 3];
                    i += 3;
                }
                else
                {
                    std::cerr << "Error: --homeassistant option requires an IP, port, and token." << std::endl;
                }
            }

            if (use_homeassistant)
            {
                std::cerr << "Using homeAssistant" << std::endl;
            }

            if (std::string(argv[i]) == "--help")
            {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  --terminal-input: Enable terminal input\n"
                          << "  --network-port <port>: Set the network port\n"
                          << "  --web-server-port <port>: Set the web server port\n"
                          << "  --threads <number>: Set the number of threads\n"
                          << "  --homeassistant <ip> <port> <token>: Enable Home Assistant integration\n"
                          << "  --help: Display this help message\n";
                return 0;
            }
        }
    }

    std::unordered_set<std::string> allowed_keys;
    allowed_keys.insert("SampleKey");
    boost::shared_ptr<authorization_api> auth = boost::make_shared<authorization_api>(allowed_keys);
    boost::shared_ptr<web_service_context> ctx = boost::make_shared<web_service_context>(threads, auth);
    std::make_shared<web_service>(
        ctx,
        getLocalIP(),
        web_server_port,
        "web_service")
        ->run();

    // Run the I/O service on the requested number of threads in a separate thread
    std::thread ioServiceThread([&ctx, thread = threads]()
                                {
                                    std::vector<std::thread> ioThreads;
                                    ioThreads.reserve(thread - 1);
                                    for (auto i = thread - 1; i > 0; --i)
                                        ioThreads.emplace_back(
                                            [&ctx]
                                            {
                                                ctx->get_ioc()->run();
                                            });

                                    ctx->get_ioc()->run();

                                    // Join all I/O threads before exiting
                                    for (auto &t : ioThreads)
                                    {
                                        if (t.joinable())
                                        {
                                            t.join();
                                        }
                                    } });

    std::cout << "Web server started." << std::endl;

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

    // Initialize the model runner with a default max_length

    ModelRunner NER_Model("./models/ner_model.tflite");
    ModelRunner Classification_Model("./models/classification_model.tflite");

    try
    {
        NER_Model.LoadTokenizer("./models/ner_tokenizer.json");
        NER_Model.LoadLabels("./models/ner_labels.json");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in the NER_Model : " << e.what() << std::endl;
        return -1;
    }

    try
    {
        Classification_Model.LoadTokenizer("./models/classification_tokenizer.json");
        Classification_Model.LoadLabels("./models/classification_type_labels.json");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in the Classification_Model : " << e.what() << std::endl;
        return -1;
    }

    NetworkManager *server = nullptr;
    try
    {
        DEBUG_PRINT("Starting NetworkManager as server.");
        server = new NetworkManager(network_port, nullptr, &NER_Model, &Classification_Model);
        networkThread = std::thread(&NetworkManager::runServer, server);
        DEBUG_PRINT("NetworkManager running.");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in the NetworkManager: " << e.what() << std::endl;
        delete server;
        return -1;
    }

    // Try to initialize Home Assistant API
    if (use_homeassistant)
    {
        try
        {
            homeAssistantAPI = std::make_unique<HomeAssistantAPI>(homeassistant_ip, homeassistant_port, homeassistant_token, server);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in the HomeAssistantAPI: " << e.what() << std::endl;
            use_homeassistant = false;
        }
    }

    TaskProcessor taskProcessor(NER_Model, Classification_Model, homeAssistantAPI.get());
    InputHandler inputHandler;
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

    if (use_terminal_input)
    {
        terminalInputThread = std::thread(terminalInputFunction, std::ref(NER_Model), std::ref(Classification_Model), homeAssistantAPI.get(), std::ref(inputHandler), std::ref(taskProcessor));
    }

    // Wait for other threads to finish
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

    if (ioServiceThread.joinable())
    {
        ioServiceThread.join();
    }

    delete server;
    std::cout << "Application finished." << std::endl;
    return 0;
}
