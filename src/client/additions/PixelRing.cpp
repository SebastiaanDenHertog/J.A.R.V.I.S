#include "PixelRing.h"
#include <stdexcept>
#include <iostream>
#include <unistd.h>

PixelRing::PixelRing(const std::string &spi_device, spi_config_t *spiConfig, int num_leds)
    : gpio(spi_device.c_str(), spiConfig), num_leds(num_leds), global_brightness(31), running(false)
{
    leds.resize(num_leds * 4, 0);
    clear();
}

PixelRing::~PixelRing()
{
    stopAnimation();
    off();
    show();
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

void PixelRing::spiWrite(const std::vector<uint8_t> &data)
{
    gpio.spi->xfer(const_cast<uint8_t *>(data.data()), data.size(), nullptr, 0);
}
