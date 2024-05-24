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

    // Check command line arguments.
    if (std::getenv("PORT") == NULL || std::getenv("THREADS") == NULL)
    {
        std::cerr << "Usage: web_service\n"
                  << "Please define the PORT and THREADS environment variables\n";
        return EXIT_FAILURE;
    }

    unsigned short web_server_port = static_cast<unsigned short>(std::atoi(std::getenv("PORT")));
    int threads = std::max<int>(1, std::atoi(std::getenv("THREADS")));

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
        return -1;
    }
    DEBUG_PRINT("Bluetooth is available.");

    BluetoothComm BluetoothComm;
    if (!BluetoothComm.initialize())
    {
        std::cerr << "Failed to initialize Bluetooth communication." << std::endl;
        return -1;
    }
    DEBUG_PRINT("Bluetooth communication initialized.");

    std::thread bluetoothThread(&BluetoothComm::handleIncomingConnectionsThread, &BluetoothComm);
    DEBUG_PRINT("Bluetooth thread started.");

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

    bluetoothThread.join();
    BluetoothComm.terminate();
    DEBUG_PRINT("Bluetooth thread joined and communication terminated.");

    std::cout << "Application finished." << std::endl;
    return 0;
}
