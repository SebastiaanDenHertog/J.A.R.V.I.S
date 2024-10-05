/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    09-05-2024
 * @Date updated    14-05-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the DeviceScanner class
 **/


#include "DeviceScanner.h"
#include <filesystem>

namespace fs = std::filesystem;

// Helper function to find the first matching file
std::string find_first_device(const std::string &directory, const std::string &prefix)
{
    for (const auto &entry : fs::directory_iterator(directory))
    {
        std::string path = entry.path().string();
        if (path.find(prefix) != std::string::npos)
        {
            return path;
        }
    }
    return "";
}

std::string DeviceScanner::get_first_spi_device(const std::string &directory)
{
    return find_first_device(directory, "spi");
}

std::string DeviceScanner::get_first_i2c_device(const std::string &directory)
{
    return find_first_device(directory, "i2c-");
}
