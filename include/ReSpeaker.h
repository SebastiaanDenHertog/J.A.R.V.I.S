// ReSpeaker.h

#ifndef RESPEAKER_H
#define RESPEAKER_H

#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <cstring>

class ReSpeaker
{
public:
  ReSpeaker(const char *device_path, uint8_t addr, uint8_t count);
  ~ReSpeaker();
  uint8_t *startCaptureAndGetAudioData(uint32_t &dataLength);
  void stopCapture();
  void setVolume(uint8_t vol);

private:
  char i2c_device[128];
  int i2c_fd;
  uint8_t device_addr;
  uint8_t mic_count;
  uint8_t databuf[2]; // Buffer for I2C data

  void initBuffer(uint8_t reg, uint8_t val);
  void sendI2C(const uint8_t *buf, uint32_t sz);
  uint32_t readI2C(uint8_t *buf, uint32_t sz);
};

#endif // RESPEAKER_H
