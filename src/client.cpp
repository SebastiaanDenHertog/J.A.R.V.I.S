#include <iostream>
#include <thread>
#include "BluetoothComm.h"
#include "Pixel_Ring.h"
#include "ReSpeaker.h"

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

int main(int argc, char *argv[])
{
#ifdef DEBUG_MODE
    std::cout << "Debug mode is ON" << std::endl;
#endif

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

    while (true)
    {
    }

    bluetoothThread.join();
    btComm.terminate();
    DEBUG_PRINT("Bluetooth thread joined and communication terminated.");

    std::cout << "Application finished." << std::endl;
    return 0;
}
