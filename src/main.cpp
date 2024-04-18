#include "ReSpeaker.h"
#include "wifi_server.h"
#include "Pixel_Ring.h"

int main(int argc, char *argv[])
{
    int port = 8080;
    PixelRing pixelring("/dev/spidev0.0", 12);
    try
    {
        const char *devicePath = "/dev/i2c-1";
        uint8_t deviceAddress = 0x40;
        uint8_t micCount = 4;

        pixelring.setBrightness(15);
        pixelring.startAnimation();

        ReSpeaker respeaker(devicePath, deviceAddress, micCount);
        wifiServer wifiserver(port, respeaker);

        wifiserver.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    pixelring.stopAnimation();
    return 0;
}
