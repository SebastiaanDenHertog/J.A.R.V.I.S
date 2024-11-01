/**
 * @Authors:            Sebastiaan den Hertog
 * @Date created:       10-10-2024
 * @Date updated:       21-10-2024 (By: Sebastiaan den Hertog)
 * @Description:        Class for handling the ethernet, TCP, and UDP connections of the ESP32.
 */

#include "Internet.h"

Preferences preferences;

bool Internet::eth_connected = false;

Internet::Internet() : useTcp(false), tcpServer(nullptr) {}

/**
 * @brief Initializes the Ethernet connection. The hostname and AP name are automatically set with a combination of "Sphere-" and the last 5 characters of the MAC address.
 */

void Internet::begin()
{
    bool useEthernet = getUsesEthernet();

    if (useEthernet)
    {
        bool ethernetInitialized = ETH.begin(phy_addr, power, mdc, mdio, type, clock_mode);

        if (ethernetInitialized)
        {
            Serial.println("Ethernet initialized, waiting for connection...");
            delay(5000); // Wait for connection to establish

            if (eth_connected && ETH.localIP() != IPAddress(0, 0, 0, 0))
            {
                Serial.println("Ethernet connected!");
                Serial.print("IP Address: ");
                Serial.println(ETH.localIP());
                return; // Ethernet connection successful, no need to continue to WiFi
            }
            else
            {
                Serial.println("Ethernet not connected or no IP assigned. Switching to WiFi.");
                eth_connected = false;
            }
        }
        else
        {
            Serial.println("Ethernet initialization failed. Switching to WiFi.");
        }
    }

    // Fallback to WiFi if Ethernet is not initialized or not connected
    if (!getSSID().isEmpty() && !getPassword().isEmpty())
    {
        Serial.println("Starting WiFi connection...");

        WiFi.begin(getSSID().c_str(), getPassword().c_str());

        int retries = 0;
        while (WiFi.status() != WL_CONNECTED && retries < 10)
        {
            delay(1000);
            retries++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nWiFi connected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
        }
        else
        {
            Serial.println("\nWiFi connection failed. No network available.");
        }
    }
    else
    {
        Serial.println("SSID or password not set. Unable to connect to WiFi.");
    }

    // Set the hostname and start AP if necessary
    String name = getControllerName();
    if (eth_connected)
    {
        ETH.setHostname(name.c_str());
        Serial.print("ETH Hostname: ");
        Serial.println(name);
    }
    else if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.setHostname(name.c_str());
        Serial.print("WiFi Hostname: ");
        Serial.println(name);
    }
    else
    {
        // Start AP mode for web configuration if needed
        WiFi.softAP(name.c_str());
        Serial.print("AP Mode Hostname: ");
        Serial.println(name);
        Serial.print("IP Address of webpage: ");
        Serial.println(WiFi.softAPIP());
    }

    applySettings();
    WiFi.onEvent(WiFiEvent);
}

/**
 * @brief Checks if Ethernet or WiFi should be used based on user preference and availability.
 * This method can be called to dynamically switch between Ethernet and WiFi if needed.
 */
void Internet::checkConnectionType()
{
    if (eth_connected && ETH.linkUp())
    {
        Serial.println("Ethernet is connected and will be used.");
    }
    else if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi is connected and will be used.");
    }
    else
    {
        Serial.println("No network is connected. Attempting to reconnect...");
        begin(); // Retry initializing the connection
    }
}

/**
 * @brief Handles the WiFi events by printing the event that has occurred
 * @param event The event that has occurred
 */
void Internet::WiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.println(ETH.localIP());
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("WiFi Connected");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("WiFi IP: ");
        Serial.println(WiFi.localIP());
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("WiFi Disconnected");
        break;
    default:
        break;
    }
}

/**
 * @brief Tests the client connection by connecting to the given host and port
 * @param host The host to connect to
 * @param port The port to connect to
 */
void Internet::testClient(const char *host, uint16_t port)
{
    Serial.print("\nconnecting to ");
    Serial.println(host);

    WiFiClient client;
    if (!client.connect(host, port))
    {
        Serial.println("connection failed");
        return;
    }
    client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
    while (client.connected() && !client.available())
        ;
    while (client.available())
    {
        Serial.write(client.read());
    }

    Serial.println("closing connection\n");
    client.stop();
}

/**
 * @brief Gets the settings from the preferences and stores them in the struct
 */
void Internet::getSettings()
{
    byte emptyArray[4] = {0, 0, 0, 0};

    byte ip[4] = {};
    byte gateway[4] = {};
    byte subnet[4] = {};
    String hostname = "";

    preferences.begin("network", false);
    if (preferences.getBytes("ip", ip, sizeof(ip)) == ESP_ERR_NOT_FOUND)
    {
        preferences.putBytes("ip", emptyArray, sizeof(emptyArray));
    }

    preferences.getBytes("gateway", gateway, sizeof(gateway));
    preferences.getBytes("subnet", subnet, sizeof(subnet));
    hostname = preferences.getString("hostname");
    preferences.end();

    // Check if the settings are not empty and set them to the struct
    if (ip != emptyArray)
    {
        networkSettings.ip = IPAddress(ip[0], ip[1], ip[2], ip[3]);
    }
    if (gateway != emptyArray)
    {
        networkSettings.gateway = IPAddress(gateway[0], gateway[1], gateway[2], gateway[3]);
    }

    if (subnet != emptyArray)
    {
        networkSettings.subnet = IPAddress(subnet[0], subnet[1], subnet[2], subnet[3]);
    }

    if (hostname != "")
    {
        networkSettings.hostname = hostname.c_str();
    }
}

/**
 * @brief Applies the settings to the Ethernet connection
 */
void Internet::applySettings()
{
    getSettings();
    bool useDHCP = isUsingDHCP();

    if (eth_connected && ETH.linkUp())
    {
        if (useDHCP)
        {
            Serial.println("Using DHCP for IP configuration (Ethernet)");
            // No need to configure IP if DHCP is used; the library handles it.
        }
        else
        {
            Serial.println("Using static IP configuration for Ethernet");
            if (!ETH.config(networkSettings.ip, networkSettings.gateway, networkSettings.subnet))
            {
                Serial.println("Failed to apply static IP configuration for Ethernet");
                return;
            }
        }
        if (!ETH.setHostname(networkSettings.hostname.c_str()))
        {
            Serial.println("Failed to set Ethernet hostname");
        }
    }
    else if (WiFi.status() == WL_CONNECTED)
    {
        if (useDHCP)
        {
            Serial.println("Using DHCP for IP configuration (WiFi)");
            // No need to configure IP if DHCP is used; the library handles it.
        }
        else
        {
            Serial.println("Using static IP configuration for WiFi");
            if (!WiFi.config(networkSettings.ip, networkSettings.gateway, networkSettings.subnet))
            {
                Serial.println("Failed to apply static IP configuration for WiFi");
            }
        }
        if (!WiFi.setHostname(networkSettings.hostname.c_str()))
        {
            Serial.println("Failed to set WiFi hostname");
        }
    }
}

/**
 * @brief Updates the settings in the preferences by getting the data from the struct. And updating the Ethernet connection
 */
void Internet::updateSettings()
{
    // Only update Ethernet settings if Ethernet is connected
    if (eth_connected && ETH.linkUp())
    {
        Serial.println("Updating settings for Ethernet...");

        byte ip[4] = {networkSettings.ip[0], networkSettings.ip[1], networkSettings.ip[2], networkSettings.ip[3]};
        byte gateway[4] = {networkSettings.gateway[0], networkSettings.gateway[1], networkSettings.gateway[2], networkSettings.gateway[3]};
        byte subnet[4] = {networkSettings.subnet[0], networkSettings.subnet[1], networkSettings.subnet[2], networkSettings.subnet[3]};

        // Store the settings in preferences
        preferences.begin("network", false);
        preferences.putBytes("ip", (byte *)(&ip), sizeof(ip));
        preferences.putBytes("gateway", (byte *)(&gateway), sizeof(gateway));
        preferences.putBytes("subnet", (byte *)(&subnet), sizeof(subnet));
        preferences.putString("hostname", networkSettings.hostname);
        preferences.end();

        // Apply the settings to the Ethernet connection
        if (!ETH.config(networkSettings.ip, networkSettings.gateway, networkSettings.subnet))
        {
            Serial.println("Failed to apply static IP configuration for Ethernet.");
        }
        if (!ETH.setHostname(networkSettings.hostname.c_str()))
        {
            Serial.println("Failed to set Ethernet hostname.");
        }
    }
    else if (WiFi.status() == WL_CONNECTED) // Only modify WiFi if it's connected
    {
        Serial.println("Updating settings for WiFi...");

        byte ip[4] = {networkSettings.ip[0], networkSettings.ip[1], networkSettings.ip[2], networkSettings.ip[3]};
        byte gateway[4] = {networkSettings.gateway[0], networkSettings.gateway[1], networkSettings.gateway[2], networkSettings.gateway[3]};
        byte subnet[4] = {networkSettings.subnet[0], networkSettings.subnet[1], networkSettings.subnet[2], networkSettings.subnet[3]};

        // Store the settings in preferences
        preferences.begin("network", false);
        preferences.putBytes("ip", (byte *)(&ip), sizeof(ip));
        preferences.putBytes("gateway", (byte *)(&gateway), sizeof(gateway));
        preferences.putBytes("subnet", (byte *)(&subnet), sizeof(subnet));
        preferences.putString("hostname", networkSettings.hostname);
        preferences.putString("ssid", networkSettings.ssid);
        preferences.putString("password", networkSettings.password);
        preferences.end();

        // Apply the settings to the WiFi connection
        if (!WiFi.config(networkSettings.ip, networkSettings.gateway, networkSettings.subnet))
        {
            Serial.println("Failed to apply static IP configuration for WiFi.");
        }
        if (!WiFi.setHostname(networkSettings.hostname.c_str()))
        {
            Serial.println("Failed to set WiFi hostname.");
        }
        if (!WiFi.begin(networkSettings.ssid.c_str(), networkSettings.password.c_str()))
        {
            Serial.println("Failed to connect to WiFi.");
        }
        Serial.write(networkSettings.ssid.c_str());
    }
    else
    {
        Serial.println("No active network connection; settings not applied.");
    }
}

/**
 * @brief Resets the settings in the preferences and sets the Ethernet connection to default settings
 */
void Internet::resetSettings()
{
    preferences.begin("network", false);
    preferences.clear();
    preferences.putString("hostname", getControllerName());
    preferences.end();

    networkSettings.ip = IPAddress(0, 0, 0, 0);
    networkSettings.gateway = IPAddress(0, 0, 0, 0);
    networkSettings.subnet = IPAddress(0, 0, 0, 0);
    networkSettings.hostname = getControllerName();

    ETH.config(networkSettings.ip, networkSettings.gateway, networkSettings.subnet);
    ETH.setHostname(networkSettings.hostname.c_str());
}

/**
 * @brief Displays the settings of the Ethernet connection for each storage method.
 * For Debugging purposes only
 */
void Internet::displaySettings()
{
    Serial.println("=====Displaying settings=====");
    Serial.println("Settings in struct:");
    Serial.print("IP: ");
    Serial.println(networkSettings.ip);
    Serial.print("Gateway: ");
    Serial.println(networkSettings.gateway);
    Serial.print("Hostname: ");
    Serial.println(networkSettings.hostname);
    Serial.print("Subnet: ");
    Serial.println(networkSettings.subnet);

    Serial.println("\nSettings in preferences:");
    byte ip[4] = {};
    byte gateway[4] = {};
    byte subnet[4] = {};
    String hostname = "";

    preferences.begin("network", false);
    preferences.getBytes("ip", ip, sizeof(ip));
    preferences.getBytes("gateway", gateway, sizeof(gateway));
    preferences.getBytes("subnet", subnet, sizeof(subnet));
    ssid = preferences.getString("ssid");
    password = preferences.getString("password");
    hostname = preferences.getString("hostname");
    useDHCP = preferences.getBool("useDHCP", true);
    preferences.end();

    Serial.print("IP: ");
    Serial.println(IPAddress(ip[0], ip[1], ip[2], ip[3]));
    Serial.print("Gateway: ");
    Serial.println(IPAddress(gateway[0], gateway[1], gateway[2], gateway[3]));
    Serial.print("Hostname: ");
    Serial.println(hostname);
    Serial.print("Subnet: ");
    Serial.println(IPAddress(subnet[0], subnet[1], subnet[2], subnet[3]));

    Serial.println("\nSettings in ETH library:");
    Serial.print("IP: ");
    Serial.println(ETH.localIP());
    Serial.print("Gateway: ");
    Serial.println(ETH.gatewayIP());
    Serial.print("Hostname: ");
    Serial.println(ETH.getHostname());
    Serial.print("Subnet: ");
    Serial.println(ETH.subnetMask());

    Serial.println("=====End of settings=====");
}

/**
 * @brief Sets the IP address of the Ethernet connection
 * @param ip The IP address to set
 */
void Internet::setIP(IPAddress ip)
{
    networkSettings.ip = ip;
    updateSettings();
}

/**
 * @brief Sets the gateway of the Ethernet connection
 * @param gateway The gateway to set
 */
void Internet::setGateway(IPAddress gateway)
{
    networkSettings.gateway = gateway;
    updateSettings();
}

/**
 * @brief Sets the subnet of the Ethernet connection
 * @param subnet The subnet to set
 */
void Internet::setSubnet(IPAddress subnet)
{
    networkSettings.subnet = subnet;
    updateSettings();
}

/**
 * @brief Sets the hostname of the Ethernet connection
 * @param hostname The hostname to set
 */
void Internet::setHostname(String hostname)
{
    networkSettings.hostname = hostname;
    updateSettings();
}

void Internet::setSSID(String ssid)
{
    networkSettings.ssid = ssid;
    preferences.begin("network", true);
    preferences.putString("ssid", ssid);
    preferences.end();
    Serial.println("SSID set to: " + ssid); // Debugging statement
}

void Internet::setPassword(String password)
{
    networkSettings.password = password;
    preferences.begin("network", true);
    preferences.putString("password", password);
    preferences.end();
    Serial.println("Password set."); // Avoid printing passwords directly for security
}

/**
 * @brief Gets the IP address of the Ethernet connection
 * @return The IP address of the Ethernet connection
 */
IPAddress Internet::getIP()
{
    if (eth_connected)
    {
        return ETH.localIP();
    }
    else if (WiFi.status() == WL_CONNECTED)
    {
        return WiFi.localIP();
    }
}

/**
 * @brief Gets the gateway of the Ethernet connection
 * @return The gateway of the Ethernet connection
 */
IPAddress Internet::getGateway()
{
    if (eth_connected)
    {
        return ETH.gatewayIP();
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        return WiFi.gatewayIP();
    }
}

/**
 * @brief Gets the subnet of the Ethernet connection
 * @return The subnet of the Ethernet connection
 */
IPAddress Internet::getSubnet()
{
    if (eth_connected)
    {
        return ETH.subnetMask();
    }
    else if (WiFi.status() == WL_CONNECTED)
    {
        return WiFi.subnetMask();
    }
}

/**
 * @brief Gets the hostname of the Ethernet connection
 * @return The hostname of the Ethernet connection
 */
const char *Internet::getHostname()
{
    if (eth_connected)
    {
        return ETH.getHostname();
    }
    else if (WiFi.status() == WL_CONNECTED)
    {
        return WiFi.getHostname();
    }
}

/**
 * @brief Gets the MAC address of the Ethernet connection
 * @return The MAC address of the Ethernet connection
 */
String Internet::getMAC()
{
    if (eth_connected)
    {
        return ETH.macAddress();
    }
    else if (WiFi.status() == WL_CONNECTED)
    {
        return WiFi.macAddress();
    }
}

/**
 * @brief Gets the link status of the Ethernet connection
 * @return The link status of the Ethernet connection
 */
bool Internet::getLinkStatus()
{
    if (eth_connected)
    {
        return ETH.linkUp();
    }
    else if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }
}

/**
 * @brief Gets the link speed of the Ethernet connection
 * @return The link speed of the Ethernet connection
 */
uint8_t Internet::getLinkSpeed()
{
    if (eth_connected)
    {
        return ETH.linkSpeed();
    }
    else if (WiFi.status() == WL_CONNECTED)
    {
        return 100;
    }
}

/**
 * @brief Initializes the connection as either UDP or TCP
 * @param port The port to use
 * @param isTcp Set to true for TCP or false for UDP
 */
void Internet::initUdpTcp(uint16_t port, bool isTcp)
{
    this->useTcp = isTcp;
    this->port = port;

    if (useTcp)
    {
        tcpServer = new WiFiServer(port);
        tcpServer->begin();
        Serial.println("TCP server started on port " + String(port));
    }
    else
    {
        udp.begin(port);
        Serial.println("UDP listener started on port " + String(port));
    }
}

/**
 * @brief Gets the controller name based on the MAC address
 */
String Internet::getControllerName()
{
    String mac = getMAC();
    String controllerName = "Jarvis-" + mac.substring(mac.length() - 5);
    return controllerName;
}

/**
 * @brief Handles incoming UDP packets
 */
void Internet::handleUdpPacket()
{
    int packetSize = udp.parsePacket();
    if (packetSize > 0)
    {
    }
}

/**
 * @brief Handles incoming TCP packets
 */
void Internet::handleTcpPacket()
{
    if (!tcpClient.connected())
    {
        tcpClient = tcpServer->available();
    }

    if (tcpClient && tcpClient.connected() && tcpClient.available())
    {
        String request = tcpClient.readStringUntil('\r');
        Serial.println(request);
    }
}

String Internet::getSSID() const
{
    return preferences.getString("ssid", "");
}

String Internet::getPassword() const
{
    return preferences.getString("password", "");
}

void Internet::setDHCP(bool useDHCP)
{
    this->useDHCP = useDHCP;
    preferences.begin("network", true);
    preferences.putBool("useDHCP", useDHCP);
    preferences.end();
}

bool Internet::isUsingDHCP() const
{
    preferences.begin("network", true);
    bool useDHCP = preferences.getBool("useDHCP", true); // Default to true (DHCP)
    preferences.end();
    return useDHCP;
}

void Internet::setEthernet(bool useEthernet)
{
    preferences.begin("network", true);
    preferences.putBool("useEthernet", useEthernet);
    preferences.end();
}

bool Internet::getUsesEthernet()
{
    preferences.begin("network", true);
    bool useEthernet = preferences.getBool("useEthernet", false); // Default to false (WiFi)
    preferences.end();
    return useEthernet;
}