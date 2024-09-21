#include "HardwareInterface.h"

// GPIO class implementation
GPIO::GPIO(const char *spiDevice, spi_config_t *spiConfig)
{
    spi = new SpiManager(spiDevice, spiConfig);
    if (!spi->begin())
    {
        delete spi;
        throw std::runtime_error("Failed to initialize SPI device");
    }
    memset(txBuffer, 0, sizeof(txBuffer));
    memset(rxBuffer, 0, sizeof(rxBuffer));
}

GPIO::~GPIO()
{
    delete spi;
}

void GPIO::setDirection(int pin, bool output)
{
    uint8_t reg = 0x00;
    uint8_t value = output ? 0xFF : 0x00;
    spiWrite(reg, value);
}

void GPIO::setValue(int pin, bool value)
{
    uint8_t reg = 0x01;
    uint8_t regValue = value ? 0xFF : 0x00;
    spiWrite(reg, regValue);
}

void GPIO::spiWrite(uint8_t reg, uint8_t value)
{
    txBuffer[0] = reg;
    txBuffer[1] = value;
    spi->xfer(txBuffer, sizeof(txBuffer), rxBuffer, sizeof(rxBuffer));
}

// SpiManager class implementation
SpiManager::SpiManager(const char *device, spi_config_t *config) : spiConfig(config)
{
    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        throw std::runtime_error("Failed to open SPI device");
    }
}

SpiManager::~SpiManager()
{
    close(fd);
}

bool SpiManager::begin()
{
    if (ioctl(fd, SPI_IOC_WR_MODE, &spiConfig->mode) < 0)
        return false;
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiConfig->bits_per_word) < 0)
        return false;
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spiConfig->speed) < 0)
        return false;
    return true;
}

void SpiManager::xfer(uint8_t *txBuf, int txLen, uint8_t *rxBuf, int rxLen)
{
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)txBuf,
        .rx_buf = (unsigned long)rxBuf,
        .len = (unsigned int)txLen,
        .speed_hz = spiConfig->speed,
        .delay_usecs = spiConfig->delay,
        .bits_per_word = spiConfig->bits_per_word,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0)
    {
        throw std::runtime_error("SPI transfer failed");
    }
}

// I2cManager class implementation
I2cManager::I2cManager(int adapter_nr, int addr) : addr(addr)
{
    char filename[20];
    snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
    file = open(filename, O_RDWR);
    if (file < 0)
    {
        throw std::runtime_error("Failed to open I2C device");
    }
    if (ioctl(file, I2C_SLAVE, addr) < 0)
    {
        throw std::runtime_error("Failed to set I2C address");
    }
}

I2cManager::~I2cManager()
{
    close(file);
}

__s32 I2cManager::readWordData(__u8 reg)
{
    __s32 res = i2c_smbus_read_word_data(file, reg);
    if (res < 0)
    {
        throw std::runtime_error("I2C read transaction failed");
    }
    return res;
}

void I2cManager::writeData(char *buf, int len)
{
    if (write(file, buf, len) != len)
    {
        throw std::runtime_error("I2C write transaction failed");
    }
}
