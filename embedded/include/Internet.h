/**
 * @Authors:            Sebastiaan den Hertog
 * @Date created:       10-10-2024
 * @Date updated:       21-10-2024 (By: Sebastiaan den Hertog)
 * @Description:        Header file for handling the internet, TCP, and UDP connections of the ESP32.
 */

#ifndef INTERNET_H
#define INTERNET_H

#include <ETH.h>
#include <WiFi.h>
#include <Preferences.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <functional>

class Internet
{
public:
    static bool eth_connected;

    Internet();
    void begin();
    void checkConnectionType();
    void testClient(const char *host, uint16_t port);
    void getSettings();
    void updateSettings();
    void applySettings();
    void displaySettings();

    void setIP(IPAddress ip);
    void setGateway(IPAddress gateway);
    void setSubnet(IPAddress subnet);
    void setHostname(String hostname);
    void setSSID(String ssid);
    void setPassword(String password);
    String getSSID() const;
    String getPassword() const;
    void resetSettings();

    IPAddress getIP();
    IPAddress getGateway();
    IPAddress getSubnet();
    const char *getHostname();
    String getMAC();
    bool getLinkStatus();
    uint8_t getLinkSpeed();
    String getControllerName();
    String ssid;
    String password;
    void setDHCP(bool useDHCP);
    bool isUsingDHCP() const;
    void setEthernet(bool useEthernet);
    bool getUsesEthernet();

    void initUdpTcp(uint16_t port, bool isTcp);
    void handleUdpPacket();
    void handleTcpPacket();
    void sendUdpPacket(const char *msg);
    void sendTcpPacket(const char *msg);

private:
    WiFiUDP udp;
    WiFiServer *tcpServer;
    WiFiClient tcpClient;

    bool useTcp;
    uint16_t port;
    bool useDHCP;
    uint8_t phy_addr = 0;
    int power = 5;
    int mdc = 23;
    int mdio = 18;
    eth_phy_type_t type = ETH_PHY_LAN8720;
    eth_clock_mode_t clock_mode = ETH_CLOCK_GPIO17_OUT;

    struct NetworkSettings
    {
        IPAddress ip = IPAddress(0, 0, 0, 0);
        IPAddress gateway = IPAddress(0, 0, 0, 0);
        IPAddress subnet = IPAddress(0, 0, 0, 0);
        String hostname = "";
        String ssid = "";
        String password = "";
    } networkSettings;

    static void WiFiEvent(WiFiEvent_t event);
};

#endif
