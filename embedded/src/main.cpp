/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     
**/

#include <Arduino.h>
#include "WebServer.h"
#include "GPIOMgr.h"
#include "TemperatureSensor.h"

// WiFi credentials
const char* ssid = ""; // environment variable
const char* password = "";  // environment variable

// Create instances of the classes
WebServer webServer(ssid, password);
GPIOMgr gpioMgr;
TemperatureSensor tempSensor(4, DHT11);  // Use GPIO 4 for the temperature sensor

void setup() {
    Serial.begin(115200);

    // Start web server
    webServer.begin();
    webServer.addGPIOConfigRoute();

    // Configure GPIO as an output example
    gpioMgr.configureGPIO(2, "output");
    gpioMgr.writeGPIO(2, HIGH);

    // Test temperature sensor
    float temperature = tempSensor.readTemperature();
    Serial.printf("Temperature: %.2f°C\n", temperature);
}

void loop() {
    // Optionally, send temperature data via WebSocket
    float temperature = tempSensor.readTemperature();
    webServer.broadcastTemperature(temperature);
    
    delay(5000);  // Broadcast every 5 seconds
}
