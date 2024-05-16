
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
    PixelRing(const std::string &i2c_device, int i2c_address, int num_leds);
    ~PixelRing();

    void startAnimation();
    void stopAnimation();
    void setBrightness(uint8_t brightness);
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void show();
    void clear();
    void off();

private:
    int i2c_fd;
    int i2c_address;
    std::vector<uint8_t> leds;
    int num_leds;
    uint8_t global_brightness;
    std::thread animationThread;
    std::atomic<bool> running;
    std::mutex lock;

    void animationLoop();
    void startFrame();
    void endFrame();
};

#endif