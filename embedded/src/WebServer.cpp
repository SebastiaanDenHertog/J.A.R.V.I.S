/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the webServer class
 */

#include "WebServer.h"

WebServer::WebServer() : server(80) {}

/**
 * @brief sets up the webserver
 */
void WebServer::begin()
{
    // Setup routes
    setupRoutes();
    updateNetwork();
    resetNetworkSettings();
    getNetworkData();

    // Starts server
    server.begin();
}

String WebServer::processor(const String& var) {
    String text;
    if(var == "CHIP_ID"){
        text = String(ESP.getEfuseMac(), HEX);
    }
    else if(var == "FLASH_CHIP_SIZE"){
        text = String(ESP.getFlashChipSize() / 1024 / 1024);
    }
    else if(var == "FLASH_CHIP_SPEED"){
        text = String(ESP.getFlashChipSpeed() / 1000000);
    }
    else if(var == "SKETCH_SIZE"){
        text = String(ESP.getSketchSize() / 1024);
    }
    else if(var == "FREE_HEAP"){
        text = String(ESP.getFreeHeap() / 1024);
    }
    else if(var == "FREE_SKETCH_SPACE"){
        text = String(ESP.getFreeSketchSpace() / 1024);
    }
    else if(var == "HOSTNAME"){
        text = internet.getHostname();
    }
    else if(var == "IP_ADDRESS"){
        text = internet.getIP().toString();
    }
    else if(var == "GATEWAY"){
        text = internet.getGateway().toString();
    }
    else if(var == "SUBNET"){
        text = internet.getSubnet().toString();
    }
    else if(var == "MAC_ADDRESS"){
        text = internet.getMAC();
    }
    else if(var == "LINK_STATUS"){
        text = internet.getLinkStatus() ? "Connected" : "Disconnected";
    }
    else if(var == "LINK_SPEED"){
        text = String(internet.getLinkSpeed());
    }
    else{
        text = "Text not found";
    }

    return text;
}

/**
 * @brief sets up the homepage route
 */
void WebServer::setupRoutes()
{
 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route for the style.css
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/style.css", "text/css");
     });
}

/**
 * @brief Updates the network settings by getting the parameters from the webpage
 */
void WebServer::updateNetwork()
{
    server.on("/updateNetwork", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  Serial.println("Updating network settings...");
                  bool settingsUpdated = false;

                  if (request->hasParam("ip")) {
                      IPAddress ip;
                      ip.fromString(request->getParam("ip")->value());
                      internet.setIP(ip);
                      settingsUpdated = true;
                  }
                  if (request->hasParam("gateway")) {
                      IPAddress gateway;
                      gateway.fromString(request->getParam("gateway")->value());
                      internet.setGateway(gateway);
                      settingsUpdated = true;
                  }
                  if (request->hasParam("subnet")) {
                      IPAddress subnet;
                      subnet.fromString(request->getParam("subnet")->value());
                      internet.setSubnet(subnet);
                      settingsUpdated = true;
                  }
                  if (request->hasParam("hostname")) {
                      internet.setHostname(request->getParam("hostname")->value());
                      settingsUpdated = true;
                  }
                  if (request->hasParam("ssid"))
                  {
                      String newSSID = request->getParam("ssid")->value();
                      internet.setSSID(newSSID);
                      settingsUpdated = true;
                  }

                  if (request->hasParam("password"))
                  {
                      String newPassword = request->getParam("password")->value();
                      internet.setPassword(newPassword);
                      settingsUpdated = true;
                  }

                  if (request->hasParam("connectionType")) {
                      String connectionType = request->getParam("connectionType")->value();
                      bool useEthernet = (connectionType == "Ethernet");
                      internet.setEthernet(useEthernet);
                      Serial.println(useEthernet ? "Ethernet selected" : "WiFi selected");
                      settingsUpdated = true;
                  }
                  if (request->hasParam("useDHCP")) {
                      bool useDHCP = request->getParam("useDHCP")->value() == "true";
                      internet.setDHCP(useDHCP);
                      Serial.println(useDHCP ? "DHCP enabled" : "Static IP enabled");
                      settingsUpdated = true;
                  }

                  if (settingsUpdated) {
                      Serial.println("Applying updated settings...");
                      internet.applySettings();

                      // Restart the connection
                      if (WiFi.status() == WL_CONNECTED) {
                          WiFi.disconnect();
                      }
                      internet.begin(); // Reinitialize connection with new settings

                      Serial.println("Network settings updated and reconnected.");
                  }
                  Serial.println("Network settings updated.");

                  request->send(200, "text/plain", "OK"); });
}

void WebServer::resetNetworkSettings()
{
    server.on("/resetNetworkSettings", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        internet.resetSettings();
        request->send(200, "text/plain", "OK"); });
}

/**
 * @brief Gets the network settings and shows them on the webpage in JSON format
 */
void WebServer::getNetworkData(){
    server.on("/getData", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Network object
        JsonObject networkData = doc["Network"].to<JsonObject>();
        networkData["IP"] = internet.getIP().toString();
        networkData["Subnet"] = internet.getSubnet();
        networkData["Gateway"] = internet.getGateway();
        networkData["Hostname"] = internet.getHostname();

        //Serialize JSON object to string
        String response;
        serializeJsonPretty(doc, response);

        //Send response
        request->send(200, "application/json", response);
    });
    
}
