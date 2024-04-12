#ifndef BLUETOOTHCOMM_H
#define BLUETOOTHCOMM_H

#include <string>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <unistd.h>
#include <sys/socket.h>

class BluetoothComm
{
public:
    BluetoothComm();
    virtual ~BluetoothComm();

    // Initialize the Bluetooth communication
    bool initialize();

    // Terminate the Bluetooth communication
    void terminate();

    // Send data over Bluetooth
    bool sendData(const std::string &data);

    // Receive data over Bluetooth
    std::string receiveData();

private:
    int sockfd; // Socket file descriptor for Bluetooth communication
};

#endif // BLUETOOTHCOMM_H
