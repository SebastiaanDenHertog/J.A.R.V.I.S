/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the PinManager class
 **/

#ifndef PINMANAGER_H
#define PINMANAGER_H

#include <Arduino.h>

class PinManager
{
private:
    int pinNumber;
    uint8_t mode;

public:
    // Constructor
    PinManager(int pin, uint8_t pinModeType);

    // Initialize the pin
    void begin();

    // Write a digital value to the pin
    void writeDigital(bool value);

    // Read a digital value from the pin
    bool readDigital();

    // Write an analog value to the pin (if supported)
    void writeAnalog(uint8_t value);

    // Read an analog value from the pin (if supported)
    uint16_t readAnalog();

    // Get the pin number
    int getPinNumber();
};

#endif // PINMANAGER_H