/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the GPIOMgr class
 */

#include "GPIOMgr.h"

void GPIOMgr::configureGPIO(int pin, const String &mode)
{
    if (mode == "input")
    {
        pinMode(pin, INPUT);
        Serial.printf("Configured GPIO %d as INPUT\n", pin);
    }
    else if (mode == "output")
    {
        pinMode(pin, OUTPUT);
        Serial.printf("Configured GPIO %d as OUTPUT\n", pin);
    }
}

int GPIOMgr::readGPIO(int pin)
{
    return digitalRead(pin);
}

void GPIOMgr::writeGPIO(int pin, int value)
{
    digitalWrite(pin, value);
}
