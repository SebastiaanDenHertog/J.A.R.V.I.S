#include <iostream>
#include <thread>
#include "BluetoothComm.h"
#include "Pixel_Ring.h"
#include "ReSpeaker.h"
#include "Wifi.h"

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

const char *devicePath = "/dev/i2c-1";
uint8_t deviceAddress = 0x3b;
uint8_t micCount = 4;
uint8_t ledCount = 12;

bool checkBluetoothAvailability();

void receiveData(wifiClient &client, int clientSd)
{
    client.session(clientSd);
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
        return -1;
    }
    DEBUG_PRINT("Bluetooth is available.");

    BluetoothComm btComm;
    if (!btComm.initialize())
    {
        std::cerr << "Failed to initialize Bluetooth communication." << std::endl;
        return -1;
    }
    DEBUG_PRINT("Bluetooth communication initialized.");

    std::thread bluetoothThread(&BluetoothComm::handleIncomingConnectionsThread, &btComm);
    DEBUG_PRINT("Bluetooth thread started.");

    PixelRing pixelring(devicePath, deviceAddress, ledCount);
    ReSpeaker respeaker(devicePath, deviceAddress, micCount);
    respeaker.initBoard();
    DEBUG_PRINT("ReSpeaker initialized.");

    pixelring.setBrightness(15);
    pixelring.startAnimation();
    DEBUG_PRINT("PixelRing animation started.");

    wifiClient client(port);

    client.setupClientSocket();

    // Set server address
    client.servAddr.sin_family = AF_INET;
    client.servAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &client.servAddr.sin_addr);

    if (client.connectToServer())
    {
        std::cout << "Connected to server at " << serverIP << std::endl;

        // Start a thread to receive data
        std::thread receiveThread(receiveData, std::ref(client), client.serverSd);
        receiveThread.detach();

        // Simulating sending data to the server
        // This part can be expanded as needed
        std::string data = "Hello, server!";
        send(client.serverSd, data.c_str(), data.length(), 0);
    }
    else
    {
        std::cerr << "Failed to connect to server at " << serverIP << std::endl;
        return 1;
    }

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    bluetoothThread.join();
    btComm.terminate();
    DEBUG_PRINT("Bluetooth thread joined and communication terminated.");

    std::cout << "Application finished." << std::endl;
    return 0;
}
