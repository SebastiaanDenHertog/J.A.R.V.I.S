#include "tensorflow/lite/interpreter.h"
#include "ReSpeaker.h"
#include "wifi_server.h"

ReSpeaker respeaker("/dev/i2c-1", 0x4b, 4);
respeaker.initBoard();
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " port" << std::endl;
        return -1;
    }
    int port = atoi(argv[1]);
    wifiServer server(port);
    server.run();
    respeaker.startCapture();
    while (true)
    {
        uint8_t audio_data = respeaker.getAudioData();
        server.sendData(&audio_data, sizeof(audio_data));
    }

    return 0;
}
