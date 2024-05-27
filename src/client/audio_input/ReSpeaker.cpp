#include "ReSpeaker.h"

// Constructor
ReSpeaker::ReSpeaker(const char *i2cDevicePath, uint8_t i2cAddr, uint8_t micCount)
    :i2c(1, i2cAddr), mic_count(micCount)
{
    // Additional initialization if necessary
}

// Destructor
ReSpeaker::~ReSpeaker()
{
    // Resources will be cleaned up by the GPIO and I2cManager destructors
}

// Send data over I2C
void ReSpeaker::sendI2C(const uint8_t *buf, uint32_t sz)
{
  std::cout << "Sending " << sz << " bytes over I2C\n";
    i2c.writeData(const_cast<char*>(reinterpret_cast<const char*>(buf)), sz);
}

// Read data from I2C
uint32_t ReSpeaker::readI2C(uint8_t *buf, uint32_t sz)
{
  std::cout << "Reading " << sz << " bytes from I2C\n";
    return i2c.readWordData(buf[0]);
}

// Initialize buffer
void ReSpeaker::initBuffer(uint8_t reg, uint8_t val)
{
  std::cout << "Initializing buffer with register 0x" << std::hex << static_cast<int>(reg) << " and value 0x" << static_cast<int>(val) << "\n";
    databuf[0] = reg;
    databuf[1] = val;
}

// Initialize board
void ReSpeaker::initBoard()
{
    uint8_t buf[1];
    if (readI2C(buf, 1) == 0)
    {
        std::cerr << "Failed to read from the device to check readiness.\n";
    }
    else
    {
        std::cout << "Device is ready for initialization.\n";
    }

    initBuffer(0x20, 0x08); // SYSCLK_CTRL register
    sendI2C(databuf, 2);
    usleep(200000); // sleep for 200ms

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

    //initBuffer(0x00, 0x12); // CHIP_RST register
    //sendI2C(databuf, 2);
    //usleep(200000); // sleep for 200ms

    if (readI2C(buf, 1) == 0)
    {
        std::cerr << "Device did not respond correctly after attempting reset.\n";
    }

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
