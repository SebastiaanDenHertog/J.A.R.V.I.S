#include "ReSpeaker.h"

// Constructor
ReSpeaker::ReSpeaker(const char *i2cDevicePath, uint8_t i2cAddr, uint8_t micCount)
    : i2c(1, i2cAddr), mic_count(micCount)
{
    memset(databuf, 0, sizeof(databuf));
    auto count = 0;
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
    i2c.writeData(const_cast<char *>(reinterpret_cast<const char *>(buf)), sz);
}

// Read data from I2C
uint32_t ReSpeaker::readI2C(uint8_t *buf, uint32_t sz)
{
    return i2c.readWordData(buf[0]);
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
    uint8_t buf[1];
    if (readI2C(buf, 1) == 0)
    {
        std::cerr << "Failed to read from the device to check readiness.\n";
    }
    else
    {
        std::cout << "Device is ready for initialization.\n";
    }
    initBuffer(0x20, 0x08);
    sendI2C(databuf, 2); // sys clock 24000000 Hz
    initBuffer(0x30, 0x35);
    sendI2C(databuf, 2); // i2s format
    initBuffer(0x30, 0xb5);
    sendI2C(databuf, 2); // i2s format
    initBuffer(0x06, 0x01);
    sendI2C(databuf, 2); // power up ac108
    initBuffer(0x07, 0x99);
    sendI2C(databuf, 2); // power up ac108
    initBuffer(0x09, 0x09);
    sendI2C(databuf, 2); // power up ac108
    initBuffer(0x32, 0x10);
    sendI2C(databuf, 2); // i2s config LRCK_POLARITY
    initBuffer(0x34, 0x0d);
    sendI2C(databuf, 2); // PCM_FORMAT
    initBuffer(0x36, 0x00);
    sendI2C(databuf, 2); // MSB first
    initBuffer(0x66, 0x00);
    sendI2C(databuf, 2); // disable high pass filter
    initBuffer(0x61, 0x0f);
    sendI2C(databuf, 2); // enable adc
    initBuffer(0xa0, 0x01);
    sendI2C(databuf, 2); // disable mic bias 1
    initBuffer(0xa7, 0x01);
    sendI2C(databuf, 2); // disable mic bias 2
    initBuffer(0xae, 0x01);
    sendI2C(databuf, 2); // disable mic bias 3
    initBuffer(0xb5, 0x01);
    sendI2C(databuf, 2); // disable mic bias 4
    initBuffer(0xa0, 0x05);
    sendI2C(databuf, 2); // disable dsm 1
    initBuffer(0xa7, 0x05);
    sendI2C(databuf, 2); // disable dsm 2
    initBuffer(0xae, 0x05);
    sendI2C(databuf, 2); // disable dsm 3
    initBuffer(0xb5, 0x05);
    sendI2C(databuf, 2); // disable dsm 4
    initBuffer(0x61, 0x1f);
    sendI2C(databuf, 2); // disable adc
    initBuffer(0xa0, 0x07);
    sendI2C(databuf, 2); // enable PGA 1
    initBuffer(0xa7, 0x07);
    sendI2C(databuf, 2); // enable PGA 2
    initBuffer(0xae, 0x07);
    sendI2C(databuf, 2); // enable PGA 3
    initBuffer(0xb5, 0x07);
    sendI2C(databuf, 2); // enable PGA 4
    initBuffer(0xba, 0x20);
    sendI2C(databuf, 2); // disable dsm globally
    initBuffer(0xbb, 0x0f);
    sendI2C(databuf, 2); // disable adc clk gating globally
    setVolume(0xde);

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
    initBuffer(0x30, 0xf0);
    sendI2C(databuf, 2); // disable global clock
    initBuffer(0x10, 0x48);
    sendI2C(databuf, 2); // disable pll
    initBuffer(0x30, 0xb0);
    sendI2C(databuf, 2); // disable all clocks
    initBuffer(0x21, 0x00);
    sendI2C(databuf, 2); // disable mod clk
    initBuffer(0x22, 0x00);
    sendI2C(databuf, 2); // disable mod rst
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
    initBuffer(0x33, 0x3f);
    sendI2C(databuf, 2); // sample resolution 16 bit
    initBuffer(0x35, 0x33);
    sendI2C(databuf, 2); // sample resolution 16 bit
    initBuffer(0x60, 0x07);
    sendI2C(databuf, 2); // sample rate 44100 Hz
    initBuffer(0x66, 0x0f);
    sendI2C(databuf, 2); // enable high pass filter

    initBuffer(0x14, 0x18);
    sendI2C(databuf, 2); // pll divider config
    initBuffer(0x13, 0x4c);
    sendI2C(databuf, 2); // pll loop divider
    initBuffer(0x12, 0x02);
    sendI2C(databuf, 2); // pll loop divider MSB
    initBuffer(0x11, 0x18);
    sendI2C(databuf, 2); // pll pre divider
    initBuffer(0x18, 0x01);
    sendI2C(databuf, 2); // pll clock lock en
    initBuffer(0x20, 0x89);
    sendI2C(databuf, 2); // pll clock source MCLK

    initBuffer(0x31, 0x03);
    sendI2C(databuf, 2); // bclk div

    initBuffer(0x38, 0x03);
    sendI2C(databuf, 2); // set channels
    initBuffer(0x39, 0x0f);
    sendI2C(databuf, 2); // tx config
    initBuffer(0x3a, 0x00);
    sendI2C(databuf, 2); // tx config
    initBuffer(0x3c, 0x4e);
    sendI2C(databuf, 2); // tx chip map
    initBuffer(0x3d, 0x00);
    sendI2C(databuf, 2); // tx chip map
    initBuffer(0x3e, 0x00);
    sendI2C(databuf, 2); // tx chip map
    initBuffer(0x3f, 0x00);
    sendI2C(databuf, 2); // tx chip map

    initBuffer(0x21, 0x93);
    sendI2C(databuf, 2); // mod clk en
    initBuffer(0x22, 0x93);
    sendI2C(databuf, 2); // mod rst ctrl

    initBuffer(0x30, 0xb0);
    sendI2C(databuf, 2); // disable all clocks
    initBuffer(0x30, 0xf0);
    sendI2C(databuf, 2); // enable lrck clock
    initBuffer(0x10, 0x4b);
    sendI2C(databuf, 2); // enable pll
    initBuffer(0x30, 0xf5);
    sendI2C(databuf, 2); // enable global clock

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
