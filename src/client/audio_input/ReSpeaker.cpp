#include "ReSpeaker.h"

ReSpeaker::ReSpeaker(const char *device_path, uint8_t addr, uint8_t count) : device_addr(addr), mic_count(count)
{
  strncpy(i2c_device, device_path, sizeof(i2c_device) - 1);
  i2c_device[sizeof(i2c_device) - 1] = '\0';

  i2c_fd = open(i2c_device, O_RDWR);
  if (i2c_fd < 0)
  {
    std::cerr << "Failed to open the I2C bus\n";
  }
}

ReSpeaker::~ReSpeaker()
{
  if (i2c_fd >= 0)
  {
    close(i2c_fd);
  }
}

void ReSpeaker::sendI2C(const uint8_t *buf, uint32_t sz)
{
  if (ioctl(i2c_fd, I2C_SLAVE, device_addr) < 0)
  {
    std::cerr << "Failed to acquire bus access and/or talk to slave.\n";
    return;
  }

  if (write(i2c_fd, buf, sz) != static_cast<ssize_t>(sz))
  {
    std::cerr << "Failed to write to the I2C bus.\n";
    return;
  }
}

uint32_t ReSpeaker::readI2C(uint8_t *buf, uint32_t sz)
{
  if (ioctl(i2c_fd, I2C_SLAVE, device_addr) < 0)
  {
    std::cerr << "Failed to acquire bus access and/or talk to slave.\n";
    return 0;
  }

  ssize_t bytes_read = read(i2c_fd, buf, sz);
  if (bytes_read != static_cast<ssize_t>(sz))
  {
    std::cerr << "Failed to read from the I2C bus.\n";
    return 0;
  }
  return static_cast<uint32_t>(bytes_read);
}

void ReSpeaker::initBuffer(uint8_t reg, uint8_t val)
{
  databuf[0] = reg;
  databuf[1] = val;
}

void ReSpeaker::initBoard()
{
  initBuffer(0x00, 0x12);
  sendI2C(databuf, 2);
  usleep(200000); // sleep for 200ms
  initBuffer(0x20, 0x08);
  sendI2C(databuf, 2);
}

void ReSpeaker::setVolume(uint8_t vol)
{
  for (uint8_t i = 0; i < mic_count; ++i)
  {
    initBuffer(0x70 + i, vol);
    sendI2C(databuf, 2);
  }
}

void ReSpeaker::stopCapture()
{
  initBuffer(0x30, 0xb0);
  sendI2C(databuf, 2);
}

uint8_t *ReSpeaker::startCaptureAndGetAudioData(uint32_t &dataLength)
{
  initBuffer(0x33, 0x7f);
  sendI2C(databuf, 2);

  dataLength = 1024;
  uint8_t *audioData = new uint8_t[dataLength];
  uint32_t bytesRead = readI2C(audioData, dataLength);
  if (bytesRead == 0)
  {
    std::cerr << "Failed to read audio data.\n";
    delete[] audioData;
    return nullptr;
  }
  return audioData;
}
