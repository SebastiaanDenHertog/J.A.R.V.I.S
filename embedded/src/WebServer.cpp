/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the webServer class
 **/

#include "WebServer.h"

WebServer::WebServer(const char *_ssid, const char *_password) : ssid(_ssid), password(_password), server(80), ws("/ws") {}

void WebServer::begin()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // WebSocket setup
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
               {
        if (type == WS_EVT_DATA) {
            // Handle incoming WebSocket data
        } });
    server.addHandler(&ws);

    // Define a basic route
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", "<h1>ESP32 Web Interface</h1>"); });

    // Start the server
    server.begin();
    Serial.println("Web server started");
}

void WebServer::addGPIOConfigRoute()
{
    server.on("/gpio", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String html = "<h1>GPIO Configuration</h1>";
        html += "<form method='POST' action='/set_gpio'>";
        html += "<label for='pin'>Pin:</label>";
        html += "<input type='number' id='pin' name='pin'><br>";
        html += "<label for='mode'>Mode:</label>";
        html += "<select id='mode' name='mode'><option value='input'>INPUT</option><option value='output'>OUTPUT</option></select><br>";
        html += "<input type='submit' value='Submit'>";
        html += "</form>";
        request->send(200, "text/html", html); });
}

void WebServer::broadcastTemperature(float temp)
{
    String message = "Temperature: " + String(temp) + "°C";
    ws.textAll(message); // Send message to all WebSocket clients
}
