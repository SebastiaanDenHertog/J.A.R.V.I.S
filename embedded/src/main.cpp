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
#include "I2SMicrophone.h"
#include "PinManager.h"

WebServer webServer;
GPIOMgr gpioMgr;
TemperatureSensor tempSensor(4, DHT11); 
I2SMicrophone mic1(I2S_NUM_0, 25, 33, 32);

void setup() {

Internet internet;
WebServer webServer;

    Serial.begin(115200);

    internet.begin();
    if (internet.getLinkStatus() || WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Network connection established.");
    }
    else
    {
        Serial.println("No network connection could be established.");
    }
    webServer.begin();

    gpioMgr.configureGPIO(2, "output");
    gpioMgr.writeGPIO(2, HIGH);

    mic1.begin();
}

void loop() {
    static unsigned long lastMicTime = 0;
    static unsigned long lastTempTime = 0;

    unsigned long currentMillis = millis();

    if (currentMillis - lastMicTime >= 10) {
        lastMicTime = currentMillis;
        mic1.readAverage();
    }

    if (currentMillis - lastTempTime >= 5000) {
        lastTempTime = currentMillis;
        float temperature = tempSensor.readTemperature();
    }
}

