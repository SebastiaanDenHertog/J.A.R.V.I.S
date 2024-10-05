/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the webServer class
 **/

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

class WebServer
{
private:
    const char *ssid;
    const char *password;
    AsyncWebServer server;
    AsyncWebSocket ws; // WebSocket for real-time communication

public:
    WebServer(const char *_ssid, const char *_password);
    void begin();
    void addGPIOConfigRoute();
    void broadcastTemperature(float temp);
};

#endif
