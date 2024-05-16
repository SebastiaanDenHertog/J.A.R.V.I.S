#ifndef DEVICESCANNER_H
#define DEVICESCANNER_H

#include <string>

class DeviceScanner
{
public:
    std::string get_first_spi_device(const std::string &directory = "/dev");

    std::string get_first_i2c_device(const std::string &directory = "/dev");
};

#endif // DEVICESCANNER_H
