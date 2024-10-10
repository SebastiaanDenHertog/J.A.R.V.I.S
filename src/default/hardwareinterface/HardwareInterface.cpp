/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    27-05-2024
 * @Date updated    29-05-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the SpiManager and gpio  class
 **/

#include "HardwareInterface.h"

/**
 * @brief Constructor for SpiManager.
 * @param device The SPI device to use.
 * @param config The SPI configuration.
 */

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

/**
 * @brief Destructor for GPIO.
 */

GPIO::~GPIO()
{
    delete spi;
}

/**
 * @brief Set the direction of a GPIO pin.
 * @param pin The pin number.
 * @param output True if the pin should be an output, false if it should be an input.
 */
void GPIO::setDirection(int pin, bool output)
{
    uint8_t reg = 0x00;
    uint8_t value = output ? 0xFF : 0x00;
    spiWrite(reg, value);
}

/**
 * @brief Set the value of a GPIO pin.
 * @param pin The pin number.
 * @param value The value to set.
 */

void GPIO::setValue(int pin, bool value)
{
    uint8_t reg = 0x01;
    uint8_t regValue = value ? 0xFF : 0x00;
    spiWrite(reg, regValue);
}

/**
 * @brief Write a value to a register over SPI.
 * @param reg The register to write to.
 * @param value The value to write.
 */

void GPIO::spiWrite(uint8_t reg, uint8_t value)
{
    txBuffer[0] = reg;
    txBuffer[1] = value;
    spi->xfer(txBuffer, sizeof(txBuffer), rxBuffer, sizeof(rxBuffer));
}

/**
 * @brief Constructor for SpiManager.
 * @param device The SPI device to use.
 * @param config The SPI configuration.
 */

SpiManager::SpiManager(const char *device, spi_config_t *config) : spiConfig(config)
{
    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        throw std::runtime_error("Failed to open SPI device");
    }
}

/**
 * @brief Destructor for SpiManager.
 */


SpiManager::~SpiManager()
{
    close(fd);
}

/**
 * @brief Initialize the SPI device.
 * @return True if successful, false otherwise.
 */

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

/**
 * @brief Perform an SPI transfer.
 * @param txBuf The transmit buffer.
 * @param txLen The transmit buffer length.
 * @param rxBuf The receive buffer.
 * @param rxLen The receive buffer length.
 */

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

/**
 * @brief Constructor for I2cManager.
 * @param adapter_nr The I2C adapter number.
 * @param addr The I2C device address.
 */

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

/**
 * @brief Destructor for I2cManager.
 */

I2cManager::~I2cManager()
{
    close(file);
}

/**
 * @brief Read a word from an I2C register.
 * @param reg The register to read from.
 * @return The value read.
 */

__s32 I2cManager::readWordData(__u8 reg)
{
    __s32 res = i2c_smbus_read_word_data(file, reg);
    if (res < 0)
    {
        throw std::runtime_error("I2C read transaction failed");
    }
    return res;
}

/**
 * @brief Write data to an I2C device.
 * @param buf The data to write.
 * @param len The length of the data.
 */

void I2cManager::writeData(char *buf, int len)
{
    if (write(file, buf, len) != len)
    {
        throw std::runtime_error("I2C write transaction failed");
    }
}
