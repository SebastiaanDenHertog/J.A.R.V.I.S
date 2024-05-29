#ifndef HARDWARE_INTERFACE_H
#define HARDWARE_INTERFACE_H

#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <spidev_lib++.h>

extern "C"
{
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
}

class SpiManager
{
public:
    SpiManager(const char *device, spi_config_t *config);
    ~SpiManager();
    bool begin();
    void xfer(uint8_t *txBuf, int txLen, uint8_t *rxBuf, int rxLen);

private:
    int fd;
    spi_config_t *spiConfig;
};

class GPIO
{
public:
    GPIO(const char *spiDevice, spi_config_t *spiConfig);
    ~GPIO();
    void setDirection(int pin, bool output);
    void setValue(int pin, bool value);
    SpiManager *spi;

private:
    uint8_t txBuffer[2];
    uint8_t rxBuffer[2];
    void spiWrite(uint8_t reg, uint8_t value);
};

class I2cManager
{
public:
    I2cManager(int adapter_nr, int addr);
    ~I2cManager();
    __s32 readWordData(__u8 reg);
    void writeData(char *buf, int len);

private:
    int file;
    int addr;
};

#endif // HARDWARE_INTERFACE_H
