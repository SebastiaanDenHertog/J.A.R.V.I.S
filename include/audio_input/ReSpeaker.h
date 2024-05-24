#ifndef RESPEAKER_H
#define RESPEAKER_H

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "Wifi.h"

class ReSpeaker
{
public:
  ReSpeaker(const char *device_path, uint8_t addr, uint8_t count);
  ~ReSpeaker();
  void initBuffer(uint8_t reg, uint8_t val);
  void initBoard();
  void setVolume(uint8_t vol);
  void stopCapture();
  uint8_t *startCaptureAndGetAudioData(uint32_t &dataLength);
  void startCaptureAndUpdateAudioData(soundData &data);

private:
  char i2c_device[128];
  int i2c_fd;
  uint8_t device_addr;
  uint8_t mic_count;
  uint8_t databuf[2];
  void sendI2C(const uint8_t *buf, uint32_t sz);
  uint32_t readI2C(uint8_t *buf, uint32_t sz);
};

#endif // RESPEAKER_H
