/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the GPIOMgr class
 **/

#ifndef GPIOMGR_H
#define GPIOMGR_H

#include <Arduino.h>

class GPIOMgr {
public:
    void configureGPIO(int pin, const String& mode);
    int readGPIO(int pin);
    void writeGPIO(int pin, int value);
};

#endif
