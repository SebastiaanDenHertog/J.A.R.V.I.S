/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     
*/

#include <Arduino.h>
#include "WebServer.h"
#include "GPIOMgr.h"
#include "TemperatureSensor.h"

const char* ssid = ""; 
const char* password = "";

WebServer webServer(ssid, password);
GPIOMgr gpioMgr;
TemperatureSensor tempSensor(4, DHT11); 

void setup() {
    Serial.begin(115200);

    webServer.begin();
    webServer.addGPIOConfigRoute();

    gpioMgr.configureGPIO(2, "output");
    gpioMgr.writeGPIO(2, HIGH);

    float temperature = tempSensor.readTemperature();
    Serial.printf("Temperature: %.2f°C\n", temperature);
}

void loop() {
    float temperature = tempSensor.readTemperature();
    webServer.broadcastTemperature(temperature);
    
    delay(5000);
}
