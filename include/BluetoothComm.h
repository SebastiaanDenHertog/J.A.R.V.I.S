#ifndef BLUETOOTHCOMM_H
#define BLUETOOTHCOMM_H

#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <thread>

struct BluetoothDevice {
    std::string address;
    std::string name;
};

class BluetoothComm
{
public:
    BluetoothComm();
    virtual ~BluetoothComm();

    // Initialize Bluetooth Low Energy communication
    bool initialize();

    // Terminate Bluetooth Low Energy communication
    void terminate();

    // Scan for nearby Bluetooth devices
    std::vector<BluetoothDevice> scanDevices();

    // List established connections
    std::vector<BluetoothDevice> listConnections();

    // Connect to a specific Bluetooth device
    bool connectToDevice(const std::string &deviceAddress);

    // Get data from a specific connection
    std::string getConnectionData(const std::string &deviceAddress);

    // Send data over Bluetooth Low Energy
    bool sendData(const std::string &data);

    // Receive data over Bluetooth Low Energy
    std::string receiveData();

    // Run in a thread to create connections
    void createConnectionsThread();

    // Run in a thread to handle incoming connection requests
    void handleIncomingConnectionsThread();

private:
    bool establishConnection();
    void closeConnection();

    int deviceHandle;     // Device handle for BLE communication
    int connectionHandle; // Connection handle for BLE communication
    std::vector<BluetoothDevice> connectedDevices; // List of connected devices
};

#endif // BLUETOOTHCOMM_H
