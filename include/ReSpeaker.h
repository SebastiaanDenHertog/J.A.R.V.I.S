#ifndef RESPEAKER_H
#define RESPEAKER_H

#include <cstdint>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

class ReSpeaker
{
public:
  ReSpeaker(const char *device_path, uint8_t addr, uint8_t count = 1);
  ~ReSpeaker();
  void initBoard();
  void setVolume(uint8_t vol);
  void startCapture();
  void stopCapture();

private:
  int i2c_fd;             // File descriptor for the I2C device
  uint8_t databuf[256];   // Buffer for I2C data
  const char *i2c_device; // I2C device file
  uint8_t device_addr;    // I2C device address
  uint8_t mic_count;
  
  void sendI2C(const uint8_t *buf, uint32_t sz);
  uint32_t readI2C(uint8_t *buf, uint32_t sz);
  void initBuffer(uint8_t reg, uint8_t val);
  uint8_t getAudioData();
};

#endif // RESPEAKER_H
