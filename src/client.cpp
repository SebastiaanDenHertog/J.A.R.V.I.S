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
std::unique_ptr<BluetoothComm> bluetoothComm;
std::thread bluetoothThread;
spi_config_t spiConfig;
std::thread AirPlayServerThread;

int AirPlayServer_port = 8080;

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

int main(int argc, char *argv[])
{
#ifdef DEBUG_MODE
    std::cout << "Debug mode is ON" << std::endl;
#endif

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <Server IP> <Port>" << std::endl;
        return 1;
    }

    const char *serverIP = argv[1];
    int port = std::stoi(argv[2]);

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
                return 1;
            }
            DEBUG_PRINT("Bluetooth communication initialized.");

            bluetoothThread = std::thread(&BluetoothComm::handleIncomingConnectionsThread, bluetoothComm.get());
            DEBUG_PRINT("Bluetooth thread started.");
        }

        DEBUG_PRINT("Starting AirPlayServer.");
        AirPlayServer airplayserver(AirPlayServer_port, "JARVIS");
        airplayserver.initialize(argc, argv);
        AirPlayServerThread = std::thread([&airplayserver, argc, argv]()
                                          { airplayserver.run(argc, argv); });
        DEBUG_PRINT("AirPlayServer started.");
        AirPlayServerThread.detach();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error initializing communication or AirPlayServer: " << e.what() << std::endl;
    }

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

        NetworkManager client(port, serverIP);
        client.connectClient();

        while (true)
        {
            try
            {
                uint32_t dataLength;
                uint8_t *audioData = respeaker.startCaptureAndGetAudioData(dataLength);
                if (audioData != nullptr)
                {
                    client.sendSoundData(audioData, dataLength);
                    delete[] audioData;
                }

                std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the frequency of sending data as needed
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error during audio data capture or sending: " << e.what() << std::endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    if (bluetoothComm)
    {
        bluetoothThread.join();
        bluetoothComm->terminate();
        DEBUG_PRINT("Bluetooth thread joined and communication terminated.");
    }

    DEBUG_PRINT("PixelRing animation stopped.");
    std::cout << "Application finished." << std::endl;
    return 0;
}
