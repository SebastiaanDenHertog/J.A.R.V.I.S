#include "BluetoothComm.h"
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <iostream>
#include <chrono>
#include <thread>

BluetoothComm::BluetoothComm() : deviceHandle(-1), connectionHandle(-1)
{
    // Constructor
}

BluetoothComm::~BluetoothComm()
{
    // Destructor
    this->terminate();
}

bool BluetoothComm::initialize()
{
    // Initialize BLE device
    this->deviceHandle = hci_open_dev(hci_get_route(nullptr));
    if (this->deviceHandle < 0)
    {
        std::cerr << "Failed to open HCI device." << std::endl;
        return false;
    }

    // Set BLE device to be discoverable
    hci_le_set_scan_enable(this->deviceHandle, 0x01, 1, 1000);

    return true;
}

void BluetoothComm::terminate()
{
    // Close the device handle if it's open
    if (this->deviceHandle != -1)
    {
        // Close the connection if it's open
        if (this->connectionHandle != -1)
        {
            this->closeConnection();
        }

        hci_close_dev(this->deviceHandle);
        this->deviceHandle = -1;
    }
}

bool BluetoothComm::establishConnection()
{
    // Placeholder example for establishing a GATT connection
    // Replace this with actual connection code
    this->connectionHandle = 1; // Simulate successful connection

    if (this->connectionHandle < 0)
    {
        std::cerr << "Failed to establish a GATT connection." << std::endl;
        return false;
    }

    return true;
}

void BluetoothComm::closeConnection()
{
    // Placeholder example for closing a GATT connection
    // Replace this with actual disconnection code
    this->connectionHandle = -1;
}

bool BluetoothComm::sendData(const std::string &data)
{
    // Check if a connection is active
    if (this->connectionHandle == -1)
    {
        std::cerr << "No active connection to send data." << std::endl;
        return false;
    }

    // Placeholder example for sending data
    // Replace this with actual data sending code
    std::cout << "Sending data: " << data << std::endl;

    return true;
}

std::string BluetoothComm::receiveData()
{
    // Check if a connection is active
    if (this->connectionHandle == -1)
    {
        std::cerr << "No active connection to receive data." << std::endl;
        return "";
    }

    // Placeholder example for receiving data
    // Replace this with actual data receiving code
    std::string receivedData = "Example received data";

    return receivedData;
}

std::vector<BluetoothDevice> BluetoothComm::scanDevices()
{
    std::vector<BluetoothDevice> devices;

    inquiry_info *ii = nullptr;
    int max_rsp = 255;
    int num_rsp;
    int dev_id = hci_get_route(nullptr);
    int sock = hci_open_dev(dev_id);
    if (dev_id < 0 || sock < 0)
    {
        std::cerr << "Failed to open socket." << std::endl;
        return devices;
    }

    ii = (inquiry_info *)malloc(max_rsp * sizeof(inquiry_info));
    num_rsp = hci_inquiry(dev_id, 8, max_rsp, nullptr, &ii, IREQ_CACHE_FLUSH);
    if (num_rsp < 0)
    {
        std::cerr << "hci_inquiry failed." << std::endl;
        free(ii);
        close(sock);
        return devices;
    }

    for (int i = 0; i < num_rsp; ++i)
    {
        char addr[19] = {0};
        char name[248] = {0};
        ba2str(&(ii+i)->bdaddr, addr);
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)
        {
            strcpy(name, "[unknown]");
        }
        devices.push_back({addr, name});
    }

    free(ii);
    close(sock);
    return devices;
}

std::vector<BluetoothDevice> BluetoothComm::listConnections()
{
    return this->connectedDevices;
}

bool BluetoothComm::connectToDevice(const std::string &deviceAddress)
{
    // Placeholder example for establishing a GATT connection to the specified device
    // Replace this with actual connection code
    BluetoothDevice device;
    device.address = deviceAddress;
    device.name = "Device Name"; // You may want to retrieve the actual name

    this->connectedDevices.push_back(device);
    this->connectionHandle = 1; // Simulate successful connection

    return true;
}

std::string BluetoothComm::getConnectionData(const std::string &deviceAddress)
{
    // Placeholder example for getting data from the specified connection
    // Replace this with actual data retrieval code
    for (const auto &device : this->connectedDevices)
    {
        if (device.address == deviceAddress)
        {
            // Simulate reading data from a characteristic
            return "Example data from " + deviceAddress;
        }
    }

    std::cerr << "No connection found for the specified device address." << std::endl;
    return "";
}



void BluetoothComm::handleIncomingConnectionsThread()
{
    while (true)
    {
        // Placeholder example for handling incoming connection requests
        // Replace this with actual handling code

        std::cout << "Handling incoming connection requests..." << std::endl;

        // Simulate incoming connection request handling
        std::this_thread::sleep_for(std::chrono::seconds(5)); // Adjust the sleep duration as needed
    }
}
