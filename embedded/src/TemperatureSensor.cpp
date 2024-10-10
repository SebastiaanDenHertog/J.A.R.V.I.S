/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the TemperatureSensor class
 */

#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(int _pin, int dhtType) : pin(_pin), dht(_pin, dhtType) {
    dht.begin();
}

float TemperatureSensor::readTemperature() {
    return dht.readTemperature();
}

float TemperatureSensor::readHumidity() {
    return dht.readHumidity();
}
