/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    12-04-2024
 * @Date updated    10-05-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the bluetooth class
 */

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

struct BluetoothDevice
{
    std::string address;
    std::string name;
};

class BluetoothComm
{
public:
    BluetoothComm();
    virtual ~BluetoothComm();

    bool initialize();
    void terminate();
    std::vector<BluetoothDevice> scanDevices();
    std::vector<BluetoothDevice> listConnections();
    bool connectToDevice(const std::string &deviceAddress);
    std::string getConnectionData(const std::string &deviceAddress);
    bool sendData(const std::string &data);
    std::string receiveData();
    void createConnectionsThread();
    void handleIncomingConnectionsThread();

private:
    bool establishConnection();
    void closeConnection();

    int deviceHandle;
    int connectionHandle;
    std::vector<BluetoothDevice> connectedDevices;
};

#endif // BLUETOOTHCOMM_H
