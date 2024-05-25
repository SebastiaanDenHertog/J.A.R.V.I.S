#ifndef PIXEL_RING_H
#define PIXEL_RING_H

#include <vector>
#include <cstdint>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

class PixelRing
{
public:
    PixelRing(const std::string &spi_device, int num_leds);
    ~PixelRing();

    void startAnimation();
    void stopAnimation();
    void setBrightness(uint8_t brightness);
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void show();
    void clear();
    void off();

private:
    int spi_fd;
    std::vector<uint8_t> leds;
    int num_leds;
    uint8_t global_brightness;
    std::thread animationThread;
    std::atomic<bool> running;
    std::mutex lock;

    void animationLoop();
    void startFrame();
    void endFrame();
    void spiOpen(const std::string &device);
    void spiClose();
    void spiWrite(const std::vector<uint8_t> &data);
};

#endif // PIXEL_RING_H
