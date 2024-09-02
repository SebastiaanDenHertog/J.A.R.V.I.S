#include <iostream>
#include <thread>
#include <memory>
#include "BluetoothComm.h"
#include "PixelRing.h"
#include "ReSpeaker.h"
#include "NetworkManager.h"
#include "HardwareInterface.h"
#include "AirPlayServer.h"

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

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
int *port;
const char *serverIP;

bool checkBluetoothAvailability()
{
    try
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
    catch (const std::exception &e)
    {
        std::cerr << "Bluetooth availability check failed: " << e.what() << std::endl;
        return false;
    }
}

void send_speech_data(NetworkManager &client)
{
    try
    {
        // Example sound data to send
        const char *soundData = "example sound data";
        client.sendSoundData(reinterpret_cast<const uint8_t *>(soundData), strlen(soundData));

        client.receiveResponse();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error sending speech data: " << e.what() << std::endl;
    }
}

void run_main_loop(AirPlayServer *server)
{
    server->main_loop();
}

int main(int argc, char *argv[])
{
#ifdef DEBUG_MODE
    std::cout << "Debug mode is ON" << std::endl;
#endif

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (std::string(argv[i]) == "--server-port")
            {
                if (i + 1 < argc)
                {
                    *port = std::stoi(argv[i+1]);
                }
            }

            if (std::string(argv[i]) == "--server-ip")
            {
                if (i + 1 < argc)
                {
                   *serverIP = argv[i + 1];
                }
            }

            if (std::string(argv[i]) == "--airplay")
            {
                use_airplay = true;
            }

            if (std::string(argv[i]) == "--bluetooth")
            {
                use_blutooth = true;
            }   
            
            if (std::string(argv[i]) == "--help")
            {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  --server-port <port>: set port to connect to main server\n"
                          << "  --server-ip <server-ip>:set ip to connect to main server \n"
                          << "  --help: Display this help message\n";
                return 0;
            }
        }
    }
  
  if (use_blutooth){
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
            NetworkManager client(*port, serverIP);
            client.connectClient();

            if (client.isConnectedToSpecialServer())
            {
                NetworkSpeechThread = std::thread(send_speech_data, std::ref(client));
                NetworkSpeechThread.detach();
            }
            std::cout << "Client finished." << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    });
    networkThread.detach();
    if(use_airplay){
        try
        {
            DEBUG_PRINT("Starting AirPlayServer.");

            // Clear argv and argc variables
            argc -= 3;
            argv += 3;
            std::cout << argv[0] << std::endl;
            AirPlayServer airplayserver;
            AirPlayServerThread = std::thread([&airplayserver, argc, argv]()
                                            {   airplayserver.run(argc, argv);
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

    hardwareThread.join();

    if (bluetoothComm)
    {
        bluetoothThread.join();
        bluetoothComm->terminate();
        DEBUG_PRINT("Bluetooth thread joined and communication terminated.");
    }

    DEBUG_PRINT("PixelRing animation stopped.");
    std::cout << "Application finished." << std::endl;

    // Wait for the AirPlayServerThread to finish
    if (AirPlayServerThread.joinable())
    {
        AirPlayServerThread.join();
    }

    return 0;
}
