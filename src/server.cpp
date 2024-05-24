#include <iostream>
#include <thread>
#include <unordered_set>
#include <cstdlib>
#include <algorithm>
#include "BluetoothComm.h"
#include "Wifi.h"
#include "model_runner.h"
#include "authorization_api.h"
#include "web_service.h"
#include <boost/make_shared.hpp>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/optional_debug_tools.h>

#ifdef DEBUG_MODE
#define DEBUG_PRINT(x) std::cout << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

int port = 8080;
bool setbluetooth = false;

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

    unsigned short web_server_port = 8081;
    int threads = 10;

    std::unordered_set<std::string> allowed_keys;
    allowed_keys.insert("SampleKey");
    boost::shared_ptr<authorization_api> auth = boost::make_shared<authorization_api>(allowed_keys);
    boost::shared_ptr<web_service_context> ctx = boost::make_shared<web_service_context>(threads, auth);

    std::make_shared<web_service>(
        ctx,
        "0.0.0.0",
        web_server_port,
        "web_service")
        ->run();

    if (!checkBluetoothAvailability())
    {
        std::cerr << "Bluetooth is not available on this device." << std::endl;
    }
    else
    {
        setbluetooth = true;
    }
    DEBUG_PRINT("Bluetooth is available.");

    std::unique_ptr<BluetoothComm> bluetoothComm;
    std::thread bluetoothThread;

    if (!setbluetooth)
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

    try
    {
        DEBUG_PRINT("Starting wifiServer.");
        wifiServer server(port);
        server.run();
        DEBUG_PRINT("wifiServer running.");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ctx]
            {
                ctx->get_ioc()->run();
            });
    ctx->get_ioc()->run();

    while (true)
    {
    }

    if (bluetoothComm)
    {
        bluetoothThread.join();
        bluetoothComm->terminate();
        DEBUG_PRINT("Bluetooth thread joined and communication terminated.");
    }

    std::cout << "Application finished." << std::endl;
    return 0;
}
