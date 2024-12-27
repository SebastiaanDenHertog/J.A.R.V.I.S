/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the webServer class
 */

#include <Wifi.h>
#include <ESPAsyncWebServer.h>
#include "Internet.h"

extern Internet internet;

class WebServer
{
public:
    WebServer();
    void begin();

private:
    AsyncWebServer server;
    const char *password;

    void setupRoutes();
    String replacePlaceholder(const String &placeholderName) const;
    void updateNetwork();
    void resetNetworkSettings();
};
