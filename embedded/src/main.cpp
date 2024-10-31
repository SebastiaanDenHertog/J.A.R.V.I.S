/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description
 **/

#include <Arduino.h>
#include "WebServer.h"
#include "PinManager.h"
#include "TemperatureSensor.h"
#include "Internet.h"

// Create instances of the classes
Internet internet;
WebServer webServer;

void setup()
{
    Serial.begin(115200);

    internet.begin();
    webServer.begin();
}

void loop()
{
    delay(1);
}
