#include <iostream>
#include <thread>
#include <fstream>
#include "BluetoothComm.h"
#include "PixelRing.h"
#include "ReSpeaker.h"
#include "Wifi.h"

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

const char *spiDevicePath = "/dev/spidev0.1";
const char *i2cDevicePath = "/dev/i2c-1";
uint8_t i2cDeviceAddress = 0x3b;
uint8_t micCount = 4;
uint8_t ledCount = 12;
bool setbluetooth = false;
std::unique_ptr<BluetoothComm> bluetoothComm;
std::thread bluetoothThread;

void setGPIOHigh()
{
    std::ofstream gpioFile("/sys/class/gpio/gpio5/value");
    if (gpioFile.is_open())
    {
        gpioFile << "1";
        gpioFile.close();
    }
    else
    {
        std::cerr << "Failed to open GPIO file." << std::endl;
    }
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

    if (!checkBluetoothAvailability())
    {
        std::cerr << "Bluetooth is not available on this device." << std::endl;
    }
    else
    {
        setbluetooth = true;
        DEBUG_PRINT("Bluetooth is available.");
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

    // Check if GPIO5 exists and set it high if it does
    if (std::ifstream("/sys/class/gpio/gpio5").good())
    {
        setGPIOHigh();
    }

    PixelRing pixelring(spiDevicePath, ledCount);
    ReSpeaker respeaker(i2cDevicePath, i2cDeviceAddress, micCount);

    try
    {
        respeaker.initBoard();
        DEBUG_PRINT("ReSpeaker initialized.");

        pixelring.setBrightness(15);
        pixelring.startAnimation();
        DEBUG_PRINT("PixelRing animation started.");

        wifiClient client(port, serverIP);

        while (true)
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
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        pixelring.stopAnimation();
        return 1;
    }

    if (bluetoothComm)
    {
        bluetoothThread.join();
        bluetoothComm->terminate();
        DEBUG_PRINT("Bluetooth thread joined and communication terminated.");
    }

    pixelring.stopAnimation();
    DEBUG_PRINT("Bluetooth thread joined and communication terminated.");

    std::cout << "Application finished." << std::endl;
    return 0;
}