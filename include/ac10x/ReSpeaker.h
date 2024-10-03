/*
Create by: Sebastiaan den Hertog
Date: 2024-09-01

Description: 
*/

#ifndef RESPEAKER_H
#define RESPEAKER_H

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include "NetworkManager.h"
#include "HardwareInterface.h"

class ReSpeaker
{
public:
    ReSpeaker(const char *i2cDevicePath, uint8_t i2cAddr, uint8_t micCount);
    ~ReSpeaker();
    void initBuffer(uint8_t reg, uint8_t val);
    void initBoard();
    void setVolume(uint8_t vol);
    void stopCapture();
    uint8_t *startCaptureAndGetAudioData(uint32_t &dataLength);
    void startCaptureAndUpdateAudioData(soundData &data);

private:
    I2cManager i2c;
    uint8_t mic_count;
    uint8_t databuf[2];
    void sendI2C(const uint8_t *buf, uint32_t sz);
    uint32_t readI2C(uint8_t *buf, uint32_t sz);
};

#endif // RESPEAKER_H
