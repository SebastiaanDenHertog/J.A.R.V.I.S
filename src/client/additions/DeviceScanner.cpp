/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    09-05-2024
 * @Date updated    14-05-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the DeviceScanner class
 **/

#include "DeviceScanner.h"
#include <dirent.h>  // For directory iteration
#include <cstring>   // For string operations
#include <string>

// Helper function to find the first matching file
std::string find_first_device(const std::string &directory, const std::string &prefix)
{
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir(directory.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            std::string path = std::string(ent->d_name);
            if (path.find(prefix) != std::string::npos)
            {
                closedir(dir);
                return directory + "/" + path;
            }
        }
        closedir(dir);
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
