#ifndef BLUETOOTHCOMM_H
#define BLUETOOTHCOMM_H

#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>

class BluetoothComm
{
public:
    BluetoothComm();
    virtual ~BluetoothComm();

    // Initialize Bluetooth Low Energy communication
    bool initialize();

    // Terminate Bluetooth Low Energy communication
    void terminate();

    // Send data over Bluetooth Low Energy
    bool sendData(const std::string &data);

    // Receive data over Bluetooth Low Energy
    std::string receiveData();

private:
    int deviceHandle;     // Device handle for BLE communication
    int connectionHandle; // Connection handle for BLE communication
};

#endif // BLUETOOTHCOMM_H
