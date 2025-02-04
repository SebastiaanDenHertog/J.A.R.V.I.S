#ifndef I2S_MICROPHONE_H
#define I2S_MICROPHONE_H

#include <driver/i2s.h>
#include <Arduino.h>

class I2SMicrophone {
private:
    i2s_port_t port;
    int wsPin;
    int sdPin;
    int sckPin;
    int bufferLen;
    int16_t* buffer;

public:
    I2SMicrophone(i2s_port_t port, int wsPin, int sdPin, int sckPin, int bufferLen = 64);
    ~I2SMicrophone();

    void begin();
    float readAverage(int rangelimit = 3000);
};

#endif // I2S_MICROPHONE_H
