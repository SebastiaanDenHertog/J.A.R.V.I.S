/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the TemperatureSensor class
 */

#include <DHT.h>

class TemperatureSensor {
private:
    int pin;
    DHT dht;

public:
    TemperatureSensor(int _pin, int dhtType);
    float readTemperature();
    float readHumidity();
};