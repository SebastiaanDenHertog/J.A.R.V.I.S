#include "PixelRing.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

PixelRing::PixelRing(const std::string &spi_device, int num_leds)
    : num_leds(num_leds), global_brightness(31), running(false)
{
    spiOpen(spi_device);
    leds.resize(num_leds * 4, 0);
    clear();
}

PixelRing::~PixelRing()
{
    stopAnimation();
    off();
    show();
    spiClose();
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
    spiWrite(leds);
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
    std::vector<uint8_t> start_frame(4, 0);
    spiWrite(start_frame);
}

void PixelRing::endFrame()
{
    std::vector<uint8_t> end_frame((num_leds + 15) / 16, 0);
    spiWrite(end_frame);
}

void PixelRing::spiOpen(const std::string &device)
{
    spi_fd = open(device.c_str(), O_RDWR);
    if (spi_fd < 0)
    {
        throw std::runtime_error("Failed to open SPI device");
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 8000000;

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0)
    {
        throw std::runtime_error("Failed to set SPI mode");
    }
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
    {
        throw std::runtime_error("Failed to set SPI bits per word");
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
    {
        throw std::runtime_error("Failed to set SPI speed");
    }
}

void PixelRing::spiClose()
{
    close(spi_fd);
}

void PixelRing::spiWrite(const std::vector<uint8_t> &data)
{
    if (write(spi_fd, data.data(), data.size()) != static_cast<ssize_t>(data.size()))
    {
        std::cerr << "Error writing to SPI device" << std::endl;
    }
}
