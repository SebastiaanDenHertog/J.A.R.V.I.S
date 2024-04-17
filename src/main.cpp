#include "ReSpeaker.h"
#include "wifi_server.h"
#include "PixelRing.h"

int main(int argc, char *argv[])
{
    int port = 8080;
    PixelRing pixel_ring("/dev/spidev0.0", 12);
    try
    {
        const char *devicePath = "/dev/i2c-1";
        uint8_t deviceAddress = 0x40;
        uint8_t micCount = 4;

        pixel_ring.setBrightness(15);
        pixel_ring.startAnimation();

        ReSpeaker respeaker(devicePath, deviceAddress, micCount);
        wifiServer server(port, respeaker);

        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    pixel_ring.stopAnimation();
    return 0;
}
