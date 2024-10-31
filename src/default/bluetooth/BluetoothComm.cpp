/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    12-04-2024
 * @Date updated    21-09-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the bluetooth class
 **/

#include "BluetoothComm.h"
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <iostream>
#include <chrono>
#include <thread>

/**
 * @brief Constructor for BluetoothComm.
 * @note This constructor initializes the device and connection handles.
 */
BluetoothComm::BluetoothComm() : deviceHandle(-1), connectionHandle(-1)
{
}

/**
 * @brief Destructor for BluetoothComm.
 * @note This destructor closes the device handle if it's open.
 */
BluetoothComm::~BluetoothComm()
{
    this->terminate();
}

/**
 * @brief Initialize the Bluetooth communication.
 * @return True if successful, false otherwise.
 */

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

/**
 * @brief Terminate the Bluetooth communication.
 * @note This method closes the device handle if it's open.
 */

void BluetoothComm::terminate()
{
    if (this->deviceHandle != -1)
    {
        if (this->connectionHandle != -1)
        {
            this->closeConnection();
        }

        hci_close_dev(this->deviceHandle);
        this->deviceHandle = -1;
    }
}

/**
 * @brief Establish a GATT connection.
 * @return True if successful, false otherwise.
 */

bool BluetoothComm::establishConnection()
{

    this->connectionHandle = 1; 

    if (this->connectionHandle < 0)
    {
        std::cerr << "Failed to establish a GATT connection." << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Close the GATT connection.
 * @note This method resets the connection handle.
 */

void BluetoothComm::closeConnection()
{
    this->connectionHandle = -1;
}

/**
 * @brief Send data over the active connection.
 * @param data The data to send.
 * @return True if successful, false otherwise.
 */

bool BluetoothComm::sendData(const std::string &data)
{
    // Check if a connection is active
    if (this->connectionHandle == -1)
    {
        std::cerr << "No active connection to send data." << std::endl;
        return false;
    }
    std::cout << "Sending data: " << data << std::endl;

    return true;
}

/**
 * @brief Receive data over the active connection.
 * @return The received data in string.
 */

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

/**
 * @brief Create a thread to handle incoming connections.
 * @note This method is a placeholder example for handling incoming connections.
 * @return a vector of Bluetooth devices
 */

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
        ba2str(&(ii + i)->bdaddr, addr);
        if (hci_read_remote_name(sock, &(ii + i)->bdaddr, sizeof(name), name, 0) < 0)
        {
            strcpy(name, "[unknown]");
        }
        devices.push_back({addr, name});
    }

    free(ii);
    close(sock);
    return devices;
}

/**
 * @brief List the connected devices.
 * @return a vector of Bluetooth devices
 */

std::vector<BluetoothDevice> BluetoothComm::listConnections()
{
    return this->connectedDevices;
}

/**
 * @brief Connect to a device with the specified address.
 * @param deviceAddress The address of the device to connect to.
 */

bool BluetoothComm::connectToDevice(const std::string &deviceAddress)
{
    BluetoothDevice device;
    device.address = deviceAddress;
    device.name = "Device Name";

    this->connectedDevices.push_back(device);
    this->connectionHandle = 1;

    return true;
}

/**
 * @brief Get data from the specified connection.
 * @param deviceAddress The address of the device to get data from.
 * @return The data from the specified connection.
 */

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

/**
 * @brief Create a thread to handle incoming connections.
 * @note This method is a placeholder example for handling incoming connections.
 */

void BluetoothComm::handleIncomingConnectionsThread()
{
    while (true)
    {

        std::this_thread::sleep_for(std::chrono::seconds(5)); // Adjust the sleep duration as needed
    }
}
