/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the PinManager class
 **/

#include "PinManager.h"

/**
 * @brief Constructor for the PinManager class
 */
PinManager::PinManager(int pin, uint8_t pinModeType)
{
    pinNumber = pin;
    mode = pinModeType;
}

/**
 * @brief Initializes the pin
 */
void PinManager::begin()
{
    pinMode(pinNumber, mode);
}

/**
 * @brief Writes a digital value to the pin
 */
void PinManager::writeDigital(bool value)
{
    digitalWrite(pinNumber, value ? HIGH : LOW);
}

/**
 * @brief Reads a digital value from the pin
 */
bool PinManager::readDigital()
{
    return digitalRead(pinNumber) == HIGH;
}

/**
 * @brief Writes an analog value to the pin
 */
void PinManager::writeAnalog(uint8_t value)
{
    analogWrite(pinNumber, value);
}

/**
 * @brief Reads an analog value from the pin
 * @return The analog value
 */
uint16_t PinManager::readAnalog()
{
    return analogRead(pinNumber);
}

/**
 * @brief Gets the pin number
 * @return The pin number
 */
int PinManager::getPinNumber()
{
    return pinNumber;
}