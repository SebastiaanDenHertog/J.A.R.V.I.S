#include "ReSpeaker.h"

// Constructor
ReSpeaker::ReSpeaker(const char *device_path, uint8_t addr, uint8_t count)
    : device_addr(addr), mic_count(count), i2c_fd(-1)
{
  strncpy(i2c_device, device_path, sizeof(i2c_device) - 1);
  i2c_device[sizeof(i2c_device) - 1] = '\0';

  i2c_fd = open(i2c_device, O_RDWR);
  if (i2c_fd < 0)
  {
    std::cerr << "Failed to open the I2C bus at " << i2c_device << "\n";
  }
  else
  {
    std::cout << "I2C bus opened successfully\n";
  }
}

// Destructor
ReSpeaker::~ReSpeaker()
{
  if (i2c_fd >= 0)
  {
    close(i2c_fd);
  }
}

// Send data over I2C
void ReSpeaker::sendI2C(const uint8_t *buf, uint32_t sz)
{
  if (ioctl(i2c_fd, I2C_SLAVE, device_addr) < 0)
  {
    std::cerr << "Failed to acquire bus access and/or talk to slave at address " << std::hex << static_cast<int>(device_addr) << ".\n";
    return;
  }

  if (write(i2c_fd, buf, sz) != static_cast<ssize_t>(sz))
  {
    std::cerr << "Failed to write to the I2C bus at address " << std::hex << static_cast<int>(device_addr) << ".\n";
    std::cerr << "Register: 0x" << std::hex << static_cast<int>(buf[0]) << ", Value: 0x" << std::hex << static_cast<int>(buf[1]) << "\n";
    std::cerr << "Error code: " << strerror(errno) << "\n"; // Log the error code
  }
  else
  {
    std::cout << "Write successful to register 0x" << std::hex << static_cast<int>(buf[0]) << " with value 0x" << std::hex << static_cast<int>(buf[1]) << "\n";
  }
}

// Read data from I2C
uint32_t ReSpeaker::readI2C(uint8_t *buf, uint32_t sz)
{
  if (ioctl(i2c_fd, I2C_SLAVE, device_addr) < 0)
  {
    std::cerr << "Failed to acquire bus access and/or talk to slave at address " << std::hex << static_cast<int>(device_addr) << ".\n";
    return 0;
  }

  ssize_t bytes_read = read(i2c_fd, buf, sz);
  if (bytes_read != static_cast<ssize_t>(sz))
  {
    std::cerr << "Failed to read from the I2C bus at address " << std::hex << static_cast<int>(device_addr) << ".\n";
    return 0;
  }
  return static_cast<uint32_t>(bytes_read);
}

// Initialize buffer
void ReSpeaker::initBuffer(uint8_t reg, uint8_t val)
{
  databuf[0] = reg;
  databuf[1] = val;
}

// Initialize board
void ReSpeaker::initBoard()
{
  // Check for device readiness before initialization
  uint8_t buf[1];
  if (readI2C(buf, 1) == 0)
  {
    std::cerr << "Failed to read from the device to check readiness.\n";
  }
  else
  {
    std::cout << "Device is ready for initialization.\n";
  }

  // Initialize system clock
  initBuffer(0x20, 0x08); // SYSCLK_CTRL register
  sendI2C(databuf, 2);
  usleep(200000); // sleep for 200ms

  // Initialize power-related registers (example sequence, adjust as necessary)
  initBuffer(0x05, 0x10); // PWR_CTRL5 register
  sendI2C(databuf, 2);
  usleep(200000); // sleep for 200ms

  initBuffer(0x06, 0x01); // PWR_CTRL6 register
  sendI2C(databuf, 2);
  usleep(200000); // sleep for 200ms

  initBuffer(0x07, 0x13); // PWR_CTRL7 register
  sendI2C(databuf, 2);
  usleep(200000); // sleep for 200ms

  initBuffer(0x09, 0x84); // PWR_CTRL9 register
  sendI2C(databuf, 2);
  usleep(200000); // sleep for 200ms

  // Attempt to reset the device
  initBuffer(0x00, 0x12); // CHIP_RST register
  sendI2C(databuf, 2);
  usleep(200000); // sleep for 200ms

  // Additional check: Verify the device is ready again
  if (readI2C(buf, 1) == 0)
  {
    std::cerr << "Device did not respond correctly after attempting reset.\n";
  }

  // Attempt to read back the register value to verify the write operation
  uint8_t readBuf[1] = {0};
  initBuffer(0x00, 0x00); // CHIP_RST register address for reading
  sendI2C(databuf, 1);    // Send register address
  if (readI2C(readBuf, 1))
  {
    std::cout << "Read back value from register 0x00: 0x" << std::hex << static_cast<int>(readBuf[0]) << "\n";
  }
  else
  {
    std::cerr << "Failed to read back value from register 0x00.\n";
  }

  std::cout << "Board initialization sequence completed.\n";
}

// Set volume
void ReSpeaker::setVolume(uint8_t vol)
{
  for (uint8_t i = 0; i < mic_count; ++i)
  {
    initBuffer(0x70 + i, vol); // ADC Digital Volume Control registers
    sendI2C(databuf, 2);
  }
}

// Stop capture
void ReSpeaker::stopCapture()
{
  initBuffer(0x30, 0xb0); // I2S_CTRL register
  sendI2C(databuf, 2);
}

// Start capture and get audio data
uint8_t *ReSpeaker::startCaptureAndGetAudioData(uint32_t &dataLength)
{
  initBuffer(0x33, 0x7f); // I2S_LRCK_CTRL2 register
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

// Start capture and update audio data
void ReSpeaker::startCaptureAndUpdateAudioData(soundData &data)
{
  initBuffer(0x33, 0x7f); // I2S_LRCK_CTRL2 register
  sendI2C(databuf, 2);

  uint32_t dataLength = 1024;
  uint8_t *audioData = new uint8_t[dataLength];
  uint32_t bytesRead = readI2C(audioData, dataLength);
  if (bytesRead == 0)
  {
    std::cerr << "Failed to read audio data.\n";
    delete[] audioData;
    return;
  }
  data.update(audioData, dataLength);
  delete[] audioData;
}
