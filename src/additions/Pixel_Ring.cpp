#include "Pixel_Ring.h"

PixelRing::PixelRing(const std::string& spi_device, int num_leds)
: num_leds(num_leds), global_brightness(31), running(false) {
    spi_fd = open(spi_device.c_str(), O_RDWR);
    if (spi_fd < 0) {
        throw std::runtime_error("Failed to open SPI device");
    }
    initSpi();
    leds.resize(num_leds * 4, 0);
    clear();
}

PixelRing::~PixelRing() {
    stopAnimation();
    off();
    show();
    close(spi_fd);
}

void PixelRing::initSpi() {
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 8000000; // 8 MHz

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0 ||
        ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        close(spi_fd);
        throw std::runtime_error("Failed to configure SPI device");
    }
}

void PixelRing::startAnimation() {
    running = true;
    animationThread = std::thread(&PixelRing::animationLoop, this);
}

void PixelRing::stopAnimation() {
    running = false;
    if (animationThread.joinable()) {
        animationThread.join();
    }
}

void PixelRing::animationLoop() {
    while (running) {
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

void PixelRing::setBrightness(uint8_t brightness) {
    std::lock_guard<std::mutex> guard(lock);
    global_brightness = std::min(brightness, uint8_t(31));
}

void PixelRing::setColor(uint8_t r, uint8_t g, uint8_t b) {
    std::lock_guard<std::mutex> guard(lock);
    for (int i = 0; i < num_leds; ++i) {
        int index = i * 4;
        leds[index] = 0xE0 | global_brightness;
        leds[index + 1] = b;
        leds[index + 2] = g;
        leds[index + 3] = r;
    }
}

void PixelRing::show() {
    std::lock_guard<std::mutex> guard(lock);
    startFrame();
    write(spi_fd, leds.data(), leds.size());
    endFrame();
}

void PixelRing::clear() {
    std::lock_guard<std::mutex> guard(lock);
    std::fill(leds.begin(), leds.end(), 0);
    for (int i = 0; i < num_leds; ++i) {
        leds[i * 4] = 0xE0 | global_brightness;
    }
}

void PixelRing::off() {
    std::lock_guard<std::mutex> guard(lock);
    clear();
}

void PixelRing::startFrame() {
    uint8_t start_frame[] = {0x00, 0x00, 0x00, 0x00};
    write(spi_fd, start_frame, sizeof(start_frame));
}

void PixelRing::endFrame() {
    uint8_t end_frame[] = {0xFF, 0xFF, 0xFF, 0xFF};
    write(spi_fd, end_frame, sizeof(end_frame));
}
