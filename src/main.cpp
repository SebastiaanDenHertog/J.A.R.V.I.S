#include <iostream>
#include <thread>
#include <memory>
#include <string>
#include <cstdio> 
#include <sstream>
#include <algorithm> 

// Common headers
#include "BluetoothComm.h"
#include "NetworkManager.h"
#include "webServer.h"
#include "ClientInfo.h"

// Debug print macro
#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

// common variables
std::thread webServerThread;
bool use_web_server = true;
unsigned short web_server_port = 15881;
bool web_server_secure = false;
std::string web_server_cert_path;
std::string web_server_key_path;
int threads = 10;
int network_port = 15880;

// common functions

// Get local IP (server-specific)
std::string getLocalIP()
{
    std::string local_ip;
    std::string cmd = "hostname -I";

    // Use unique_ptr for automatic pipe cleanup
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        std::cerr << "popen() failed!" << std::endl;
        return "";
    }

    // Read from the pipe
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
    {
        local_ip += buffer;
    }

    // Remove newline character if it exists
    local_ip.erase(std::remove(local_ip.begin(), local_ip.end(), '\n'), local_ip.end());

    // Extract the first IP address from the result
    std::stringstream ss(local_ip);
    std::string first_ip;
    ss >> first_ip;

    return first_ip;
}

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

#ifdef CLIENT_BUILD OR FULL_BUILD
// Client-specific headers
#include "PixelRing.h"
#include "ReSpeaker.h"
#include "HardwareInterface.h"
#include "AirPlayServer.h"

// Client-specific variables
const char *spiDevicePath = "/dev/spidev0.0";
const char *i2cDevicePath = "/dev/i2c-1";
uint8_t i2cDeviceAddress = 0x3b;
uint8_t micCount = 4;
uint8_t ledCount = 16;
bool setbluetooth = false;
bool use_airplay = false;
bool use_blutooth = false;
std::unique_ptr<BluetoothComm> bluetoothComm;
std::thread bluetoothThread;
spi_config_t spiConfig;
std::thread AirPlayServerThread;
std::thread NetworkSpeechThread;
int server_port;
const char *serverIP;

// Send speech data (client-specific)
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

// Run main loop for AirPlay (client-specific)
void run_main_loop(AirPlayServer *server)
{
    server->main_loop();
}

ClientInfo device{"client", getLocalIP(), network_port, {}};

#endif // CLIENT_BUILD

#ifdef SERVER_BUILD OR FULL_BUILD
// Server-specific headers
#include <unordered_set>
#include <cstdlib>
#include <algorithm>
#include <chrono>
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

// Server-specific variables and functions
std::thread bluetoothThread;
std::thread networkThread;
std::thread terminalInputThread;
std::thread homeAssistantThread;
std::thread taskProcessingThread;

std::unique_ptr<BluetoothComm> bluetoothComm;
std::unique_ptr<HomeAssistantAPI> homeAssistantAPI;
NetworkManager *networkserver = nullptr;
std::vector<std::thread> io_threads;

int homeassistant_port = 0;
bool setbluetooth = false;
bool use_homeassistant = false;
bool use_terminal_input = false;

std::string homeassistant_ip;
std::string homeassistant_token;

ClientInfo device{"server", getLocalIP(), network_port, {}};

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

#endif // SERVER_BUILD

int main(int argc, char *argv[])
{
#ifdef DEBUG_MODE
    std::cout << "Debug mode is ON" << std::endl;
#endif

#ifdef CLIENT_BUILD OR FULL_BUILD

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (std::string(argv[i]) == "-server-port")
            {
                if (i + 1 < argc)
                {
                    server_port = std::atoi(argv[i + 1]);
                }
            }

            if (std::string(argv[i]) == "-server-ip")
            {
                if (i + 1 < argc)
                {
                    serverIP = argv[i + 1];
                }
            }

            if (std::string(argv[i]) == "-start-web-server")
            {
                use_web_server = true;
            }

            if (std::string(argv[i]) == "-web-server-port")
            {
                if (i + 1 < argc)
                {
                    web_server_port = static_cast<unsigned short>(std::atoi(argv[i + 1]));
                }
            }

            if (std::string(argv[i]) == "-airplay")
            {
                use_airplay = true;
            }

            if (std::string(argv[i]) == "-bluetooth")
            {
                use_blutooth = true;
            }

            if (std::string(argv[i]) == "-help")
            {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  -server-port <port>      : Set port to connect to the main server.\n"
                          << "  -server-ip <server-ip>   : Set IP address to connect to the main server.\n"
                          << "  -airplay                 : Enable AirPlay server functionality.\n"
                          << "  -bluetooth               : Enable Bluetooth communication functionality.\n"
                          << "  -help                    : Display this help message.\n"
                          << "AirPlay Options:\n"
                          << "  -allow <client>          : Allow specified client for AirPlay.\n"
                          << "  -block <client>          : Block specified client from AirPlay.\n"
                          << "  -restrict                : Restrict clients for AirPlay (use 'no' to disable).\n"
                          << "  -n <name>                : Set the server name for AirPlay.\n"
                          << "  -nh                      : Disable appending hostname for AirPlay.\n"
                          << "  -async [no] <delay>      : Enable audio synchronization (or disable with 'no') with optional delay.\n"
                          << "  -vsync [no] <delay>      : Enable video synchronization (or disable with 'no') with optional delay.\n"
                          << "  -s <widthxheight>@<rate> : Set display resolution and refresh rate for video output.\n"
                          << "  -fps <n>                 : Set the frame rate (default is 30fps).\n"
                          << "  -o                       : Enable overlay mode for video.\n"
                          << "  -f <flip-type>           : Set video flip type (options: H, V, I).\n"
                          << "  -r <rotation-type>       : Set video rotation type (options: R, L).\n"
                          << "  -p [tcp|udp]             : Set the protocol for communication (default is tcp and udp).\n"
                          << "  -m <mac-address>         : Set MAC address (or use random if omitted).\n"
                          << "  -a                       : Disable audio functionality.\n"
                          << "  -d                       : Toggle debug logging.\n"
                          << "  -fs                      : Enable fullscreen mode for video.\n"
                          << "  -reset <n>               : Set the reset timeout (n >= 0, default is 0).\n"
                          << "  -nofreeze                : Do NOT leave frozen screen in place after reset.\n"
                          << "  -key <filename>          : Specify the file for persistent key storage.\n"
                          << "  -pin <nnnn>              : Enable legacy pairing with the specified 4-digit PIN.\n"
                          << "  -reg <filename>          : Specify the file for AirPlay registration data.\n"
                          << "  -as <audiosink>          : Set the audio sink for AirPlay.\n"
                          << "  -vs <videosink>          : Set the video sink for AirPlay.\n"
                          << "  -nc                      : Set new window closing behavior.\n"
                          << "  -avdec                   : Use avdec decoder for video.\n"
                          << "  -v4l2                    : Use v4l2 decoder for video on Linux devices.\n"
                          << "  -db <low:high>           : Set decibel gain range for audio.\n"
                          << std::endl;
                return 0;
            }
        }
    }

    if (use_web_server)
    {
        webServerThread = std::thread(setup_server, web_server_secure, web_server_cert_path, web_server_key_path, web_server_port, threads);
        DEBUG_PRINT("Web service is running in the background");
    }

    if (use_blutooth)
    {
        try
        {

            if (checkBluetoothAvailability())
            {
                setbluetooth = true;
                DEBUG_PRINT("Bluetooth is available.");
            }
            else
            {
                std::cerr << "Bluetooth is not available on this device." << std::endl;
            }

            if (setbluetooth)
            {
                bluetoothComm = std::make_unique<BluetoothComm>();
                if (!bluetoothComm->initialize())
                {
                    std::cerr << "Failed to initialize Bluetooth communication." << std::endl;
                }
                DEBUG_PRINT("Bluetooth communication initialized.");

                bluetoothThread = std::thread(&BluetoothComm::handleIncomingConnectionsThread, bluetoothComm.get());
                DEBUG_PRINT("Bluetooth thread started.");
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error initializing communication or AirPlayServer: " << e.what() << std::endl;
        }
    }

    std::thread networkThread([&]()
                              {
        try
        {
            std::cout << "Client started." << std::endl;
            NetworkManager client(
                server_port, serverIP, NetworkManager::Protocol::TCP);
            client.connectClient();


                NetworkSpeechThread = std::thread(send_speech_data, std::ref(client));
                NetworkSpeechThread.detach();
            
            std::cout << "Client finished." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        } });
    networkThread.detach();
    if (use_airplay)
    {
        try
        {
            DEBUG_PRINT("Starting AirPlayServer.");
            AirPlayServer airplayserver;

            // Pass all arguments as-is to parse_arguments
            airplayserver.parse_arguments(argc, argv);

            AirPlayServerThread = std::thread([&airplayserver]()
                                              {
                airplayserver.run();
                airplayserver.main_loop(); });

            DEBUG_PRINT("AirPlayServer started.");
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error initializing communication or AirPlayServer: " << e.what() << std::endl;
            return -1;
        }
    }
    std::thread hardwareThread([&]()
                               {
        try
        {
            spiConfig.mode = 0;
            spiConfig.speed = 8000000;
            spiConfig.delay = 0;
            spiConfig.bits_per_word = 8;

            GPIO gpio(spiDevicePath, &spiConfig);
            gpio.setDirection(5, true);
            gpio.setValue(5, true);
            std::cerr << "Successfully set GPIO5 high." << std::endl;

            ReSpeaker respeaker(i2cDevicePath, i2cDeviceAddress, micCount);
            PixelRing pixelring(spiDevicePath, &spiConfig, ledCount);

            respeaker.initBoard();
            DEBUG_PRINT("ReSpeaker initialized.");

            pixelring.setBrightness(15);
            pixelring.startAnimation();
            DEBUG_PRINT("PixelRing animation started.");
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return -1;
        } });

    std::cout << "Press Ctrl+C to exit...\n";
    hardwareThread.join();

    if (bluetoothComm)
    {
        bluetoothThread.join();
        bluetoothComm->terminate();
        DEBUG_PRINT("Bluetooth thread joined and communication terminated.");
    }

    // Wait for the AirPlayServerThread to finish

    if (AirPlayServerThread.joinable())
    {
        AirPlayServerThread.join();
    }
    if (webServerThread.joinable())
    {
        webServerThread.join();
    }

    return 0;

#endif // CLIENT_BUILD

#ifdef SERVER_BUILD OR FULL_BUILD
    // Process server-specific arguments

    auto registry = std::make_shared<prometheus::Registry>();
    auto &PHstatusset = prometheus::BuildCounter()
                            .Name("api_status")
                            .Help("Returns the status of the API (1 if up, 0 if down)")
                            .Register(*registry);

    auto &PHstatus = PHstatusset.Add({{"api", "status"}, {"status", "up"}});
    PHstatus.Increment();

    // Handle input arguments
    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (std::string(argv[i]) == "-terminal-input")
            {
                use_terminal_input = true;
            }

            if (std::string(argv[i]) == "-network-port")
            {
                if (i + 1 < argc)
                {
                    network_port = std::atoi(argv[i + 1]);
                    device.setPort(network_port);
                }
            }
            if (std::string(argv[i]) == "-start-web-server")
            {
                use_web_server = true;
            }

            if (std::string(argv[i]) == "-web-server-port")
            {
                if (i + 1 < argc)
                {
                    web_server_port = static_cast<unsigned short>(std::atoi(argv[i + 1]));
                }
            }

            if (std::string(argv[i]) == "-web-server-secure")
            {
                web_server_secure = true;
                if (i + 2 < argc)
                {
                    web_server_cert_path = argv[i + 1];
                    web_server_key_path = argv[i + 2];
                    i += 2;
                }
                else
                {
                    std::cerr << "Error: --web-server-secure option requires a certificate path and key path." << std::endl;
                    return -1;
                }
            }

            if (std::string(argv[i]) == "-threads")
            {
                if (i + 1 < argc)
                {
                    threads = std::max<int>(1, std::atoi(argv[i + 1]));
                }
            }

            if (std::string(argv[i]) == "-homeassistant")
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

            if (std::string(argv[i]) == "-help")
            {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  -terminal-input: Enable terminal input\n"
                          << "  -network-port <port>: Set the network port\n"
                          << "  -web-server-port <port>: Set the web server port\n"
                          << "  -threads <number>: Set the number of threads\n"
                          << "  -homeassistant <ip> <port> <token>: Enable Home Assistant integration\n"
                          << "  -start-web-server: Start the web server\n"
                          << "  -web-server-secure <cert> <key>: Start the web server with SSL using the provided certificate and key\n"
                          << "  -help: Display this help message\n";
                return 0;
            }
        }
    }

    if (use_web_server)
    {
        webServerThread = std::thread(setup_server, web_server_secure, web_server_cert_path, web_server_key_path, web_server_port, threads);
        DEBUG_PRINT("Web service is running in the background");
    }

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

    // Initialize the model runners
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

    try
    {
        DEBUG_PRINT("Starting NetworkManager as server.");
        networkserver = new NetworkManager(network_port, nullptr, NetworkManager::Protocol::TCP, &NER_Model, &Classification_Model);
        networkThread = std::thread(&NetworkManager::runServer, networkserver);
        DEBUG_PRINT("NetworkManager running.");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in the NetworkManager: " << e.what() << std::endl;
        delete networkserver;
        return -1;
    }

    // Try to initialize Home Assistant API
    if (use_homeassistant)
    {
        try
        {
            homeAssistantAPI = std::make_unique<HomeAssistantAPI>(homeassistant_ip, homeassistant_port, homeassistant_token, networkserver);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in the HomeAssistantAPI: " << e.what() << std::endl;
            use_homeassistant = false;
        }
    }

    TaskProcessor taskProcessor(homeAssistantAPI.get(), NER_Model, Classification_Model);
    InputHandler inputHandler;

    // Process tasks in the background
    try
    {
        taskProcessingThread = std::thread([&]()
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
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in taskProcessingThread: " << e.what() << std::endl;
    }

    if (use_terminal_input)
    {
        terminalInputThread = std::thread(terminalInputFunction, std::ref(NER_Model), std::ref(Classification_Model), homeAssistantAPI.get(), std::ref(inputHandler), std::ref(taskProcessor));
    }

    // Wait for threads to join on exit
    std::cout << "Press Ctrl+C to exit...\n";

    if (bluetoothThread.joinable())
    {
        bluetoothThread.join();
    }

    if (networkThread.joinable())
    {
        networkThread.join();
    }

    if (taskProcessingThread.joinable())
    {
        taskProcessingThread.join();
    }

    if (webServerThread.joinable())
    {
        webServerThread.join();
    }

    delete networkserver;
    std::cout << "Application finished." << std::endl;
    return 0;
#endif // SERVER_BUILD
}
