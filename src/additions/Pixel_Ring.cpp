#include "Pixel_Ring.h"
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>

PixelRing::PixelRing(const std::string &i2c_device, int i2c_address, int num_leds)
    : num_leds(num_leds), global_brightness(31), running(false), i2c_address(i2c_address)
{
    i2c_fd = open(i2c_device.c_str(), O_RDWR);
    if (i2c_fd < 0)
    {
        throw std::runtime_error("Failed to open I2C device");
    }
    if (ioctl(i2c_fd, I2C_SLAVE, i2c_address) < 0)
    {
        close(i2c_fd);
        throw std::runtime_error("Failed to set I2C address");
    }
    leds.resize(num_leds * 4, 0);
    clear();
}

PixelRing::~PixelRing()
{
    stopAnimation();
    off();
    show();
    close(i2c_fd);
}

void PixelRing::startAnimation()
{
    running = true;
    animationThread = std::thread(&PixelRing::animationLoop, this);
}

void PixelRing::stopAnimation()
{
    running = false;
    if (animationThread.joinable())
    {
        animationThread.join();
    }
}

void PixelRing::animationLoop()
{
    while (running)
    {
        std::lock_guard<std::mutex> guard(lock);
        // Example animation: cycle colors
        setColor(255, 0, 0); // Red
        show();
        sleep(1);

        setColor(0, 255, 0); // Green
        show();
        sleep(1);

        setColor(0, 0, 255); // Blue
        show();
        sleep(1);
    }
    off();
    show();
}

void PixelRing::setBrightness(uint8_t brightness)
{
    std::lock_guard<std::mutex> guard(lock);
    global_brightness = std::min(brightness, uint8_t(31));
}

void PixelRing::setColor(uint8_t r, uint8_t g, uint8_t b)
{
    std::lock_guard<std::mutex> guard(lock);
    for (int i = 0; i < num_leds; ++i)
    {
        int index = i * 4;
        leds[index] = 0xE0 | global_brightness;
        leds[index + 1] = b;
        leds[index + 2] = g;
        leds[index + 3] = r;
    }
}

void PixelRing::show()
{
    std::lock_guard<std::mutex> guard(lock);
    startFrame();
    if (write(i2c_fd, leds.data(), leds.size()) != leds.size())
    {
        std::cerr << "Error writing to I2C device" << std::endl;
    }
    endFrame();
}

void PixelRing::clear()
{
    std::lock_guard<std::mutex> guard(lock);
    std::fill(leds.begin(), leds.end(), 0);
    for (int i = 0; i < num_leds; ++i)
    {
        leds[i * 4] = 0xE0 | global_brightness;
    }
}

void PixelRing::off()
{
    std::lock_guard<std::mutex> guard(lock);
    clear();
}

void PixelRing::startFrame()
{
    // Some LED protocols require special frames at the start.
    // Adapt this function if necessary based on your LED protocol.
}

void PixelRing::endFrame()
{
    // Some LED protocols require special frames at the end.
    // Adapt this function if necessary based on your LED protocol.
}