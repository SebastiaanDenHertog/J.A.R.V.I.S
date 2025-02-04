/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the webServer class
 */

#include "WebServer.h"

extern Internet internet;

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

    // Starts server
    server.begin();
}

/**
 * @brief sets up the homepage route
 */
void WebServer::setupRoutes()
{
 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route for the api.js
    server.on("/api.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/api.js", "text/javascript");
    });

    // Route for the style.css
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/style.css", "text/css");
     });

    // Route for bootstrap css
    server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/bootstrap.min.css", "text/css");
    });

    // Route for ledstrip.js
    server.on("/ledstrip.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/ledstrip.js", "text/javascript");
    });

    // Route for createHTML.js
    server.on("/createHTML.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/createHTML.js", "text/javascript");
    });

    // Route for globals.js 
    server.on("/globals.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/globals.js", "text/javascript");
    });
}

/**
 * @brief Replaces the placeholder with content
 * @param placeholderName The placeholder to be selected
 * @return Corresponding HTML content that has to be replaced
 */
String WebServer::replacePlaceholder(const String &placeholderName) const
{
    // Displays the Chip information
    if (placeholderName == "SYSTEM_INFO_PLACEHOLDER")
    {
        String systemInfo = "";
        systemInfo += "<table>";
        systemInfo += "<tr><td>Chip ID:</td>            <td>" + String(ESP.getEfuseMac(), HEX) + "</td></tr>";
        systemInfo += "<tr><td>Flash Chip Size:</td>    <td>" + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB</td></tr>";
        systemInfo += "<tr><td>Flash Chip Speed:</td>   <td>" + String(ESP.getFlashChipSpeed() / 1000000) + " MHz</td></tr>";
        systemInfo += "<tr><td>Sketch Size:</td>        <td>" + String(ESP.getSketchSize() / 1024) + " KB</td></tr>";
        systemInfo += "<tr><td>Free Heap:</td>          <td>" + String(ESP.getFreeHeap() / 1024) + " KB</td></tr>";
        systemInfo += "<tr><td>Free Sketch Space:</td>  <td>" + String(ESP.getFreeSketchSpace() / 1024) + " KB</td></tr>";
        systemInfo += "</table>";
        return systemInfo;
    }
    // Displays network information
    else if (placeholderName == "NETWORK_INFO_PLACEHOLDER")
    {
        String networkInfo = "";
        networkInfo += "<table>";
        networkInfo += "<tr><td>Hostname:</td>      <td>" + String(internet.getHostname()) + "</td></tr>";
        networkInfo += "<tr><td>IP:</td>            <td>" + internet.getIP().toString() + "</td></tr>";
        networkInfo += "<tr><td>Gateway:</td>       <td>" + internet.getGateway().toString() + "</td></tr>";
        networkInfo += "<tr><td>Subnet:</td>        <td>" + internet.getSubnet().toString() + "</td></tr>";
        networkInfo += "<tr><td>MAC:</td>           <td>" + internet.getMAC() + "</td></tr>";
        networkInfo += "<tr><td>Link status:</td>   <td>" + String(internet.getLinkStatus() ? "Connected" : "Disconnected") + "</td></tr>";
        networkInfo += "<tr><td>Link speed:</td>    <td>" + String(internet.getLinkSpeed()) + " Mbps</td></tr>";
        networkInfo += "</table>";
        return networkInfo;
    }
    else if (placeholderName == "UPDATE_NETWORK_PLACEHOLDER")
    {
        String network = "";
        network += "<form onsubmit=\"event.preventDefault(); updateNetwork(document.getElementById('ip').value, document.getElementById('gateway').value, document.getElementById('subnet').value, document.getElementById('hostname').value, document.getElementById('ssid').value, document.getElementById('password').value, document.getElementById('connectionType').value)\">";
        network += "<table>";
        network += "<tr><td>IP:</td>        <td><input type=\"text\" id=\"ip\" placeholder=\"Enter IP\" value=\"192.168.1.2\"></td></tr>";
        network += "<tr><td>Gateway:</td>   <td><input type=\"text\" id=\"gateway\" placeholder=\"Enter Gateway\" value=\"192.168.1.1\"></td></tr>";
        network += "<tr><td>Subnet:</td>    <td><input type=\"text\" id=\"subnet\" placeholder=\"Enter Subnet\" value=\"255.255.255.0\"></td></tr>";
        network += "<tr><td>Hostname:</td>  <td><input type=\"text\" id=\"hostname\" placeholder=\"Enter Hostname\"></td></tr>";
        network += "<tr><td>Use DHCP:</td> <td><input type=\"checkbox\" id=\"useDHCP\" " + String(internet.isUsingDHCP() ? "checked" : "") + "></td></tr>";
        network += "<tr><td>SSID:</td>      <td><input type=\"text\" id=\"ssid\" placeholder=\"Enter SSID\" value=\"" + internet.getSSID() + "\"></td></tr>";
        network += "<tr><td>Password:</td>  <td><input type=\"password\" id=\"password\" placeholder=\"Enter Password\" value=\"" + internet.getPassword() + "\"></td></tr>";
        network += "<tr><td>Connection Type:</td> <td><select id=\"connectionType\"><option value=\"WiFi\">WiFi</option><option value=\"Ethernet\">Ethernet</option></select></td></tr>";
        network += "</table>";
        network += "<button type=\"submit\">Submit</button>";
        network += "<script>document.querySelector('form').addEventListener('submit', function() { setTimeout(function() { location.reload(); }, 1000); document.getElementById('submitMessage').style.display = 'block'; });</script>";
        network += "<p id='submitMessage' style='color: green; display: none;'>Settings submitted</p>";
        network += "</form>";
        return network;
    }

    else if (placeholderName == "CONTROLLER_NAME")
    {
        return internet.getControllerName();
    }
    else if (placeholderName == "RESET_NETWORK_PLACEHOLDER")
    {
        String resetNetworkSettings = "";
        resetNetworkSettings += "<button type=\"submit\" onclick=\"resetNetworkSettings()\">Reset network settings</button>";
        return resetNetworkSettings;
    }

    else
    {
        return "Error displaying field and/or data";
    }
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