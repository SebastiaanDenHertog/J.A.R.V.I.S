/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description
 */

#include <Arduino.h>
#include "WebServer.h"
#include "PinManager.h"

Internet internet;
WebServer webServer;

void setup()
{
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
}

void loop()
{
    delay(1);
}
