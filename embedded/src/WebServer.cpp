/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    04-10-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the webServer class
 */

#include "WebServer.h"

WebServer::WebServer() : server(80) {}

/**
 * @brief HTML webpage
 */
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
        <title>%CONTROLLER_NAME%</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="icon" href="data:,">
        <style>
            body { text-align: center; font-family: "Trebuchet MS", Arial; margin-left:auto; margin-right:auto; }
            h1 { color: #0f3376; }
            h2 { color: #0f3376; }
            h3 { color: #0f3376; }
            .card { background-color: #f7f7f7; border-radius: 10px; padding: 20px; margin: 20px; display: inline-block; }
            table { margin-left:auto; margin-right:auto; text-align: left; }
            th, td { text-align: left; }
            input, select { width: 100%; padding: 12px 20px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; box-sizing: border-box; }
            button { background-color: #0f3376; color: white; padding: 14px 20px; margin: 8px 0; border: none; cursor: pointer; width: 100%; }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>Sphere Controller</h1>
            <h2>(%CONTROLLER_NAME%)</h2>
            
            <div class="card">
                <h3>Update Network Settings</h3>
                %UPDATE_NETWORK_PLACEHOLDER%
            </div>
            
            <div class="card">
                <h3>Network Info</h3>
                %NETWORK_INFO_PLACEHOLDER%
            </div>

            <div class="card">
                <h3>System Info</h3>
                %SYSTEM_INFO_PLACEHOLDER%
            </div>

            <br>
            <div class="card">
                <h3 style="color:red;">Reset Network Settings</h3>
                %RESET_NETWORK_PLACEHOLDER%
            </div>
        </div>

        <script>
            function updateNetwork(ip, gateway, subnet, hostname, ssid, password, connectionType, useDHCP){
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "updateNetwork?ip=" + ip + "&gateway=" + gateway + "&subnet=" + subnet + "&hostname=" + hostname + "&ssid=" + ssid + "&password=" + password + "&connectionType=" + connectionType + "&useDHCP=" + useDHCP, true);
                xhr.onload = function() {
                    if (xhr.status == 200) {
                        alert("Network settings updated successfully. The device will restart the connection.");
                    }
                };
                xhr.send();
            }
            function resetNetworkSettings(){
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "resetNetworkSettings", true);
                xhr.onload = function(){
                    if(xhr.status == 200){
                        location.reload();
                    }
                }
                xhr.send();
            }
        </script>
    </body>
</html>
)rawliteral";

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
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html, [this](const String &var)
                                { return this->replacePlaceholder(var); }); });
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
        String resetNetworkSettingsButton = "";
        resetNetworkSettingsButton += "<button type=\"submit\" onclick=\"resetNetworkSettings()\">Reset network settings</button>";
        return resetNetworkSettingsButton;
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