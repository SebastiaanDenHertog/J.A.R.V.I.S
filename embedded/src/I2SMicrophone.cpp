/**
 * @Authors:            Sebastiaan den Hertog
 * @Date created:       10-12-2024
 * @Date updated:       10-12-2024 (By: Sebastiaan den Hertog)
 * @Description:        Class for handling the i2s mics.
 */

#include "I2SMicrophone.h"

I2SMicrophone::I2SMicrophone(i2s_port_t port, int wsPin, int sdPin, int sckPin, int bufferLen)
    : port(port), wsPin(wsPin), sdPin(sdPin), sckPin(sckPin), bufferLen(bufferLen) {
    buffer = new int16_t[bufferLen];
}

I2SMicrophone::~I2SMicrophone() {
    delete[] buffer;
    i2s_driver_uninstall(port);
}

void I2SMicrophone::begin() {
    // Set up I2S configuration
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 44100,
        .bits_per_sample = i2s_bits_per_sample_t(16),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = bufferLen,
        .use_apll = false
    };

    // Install I2S driver
    i2s_driver_install(port, &i2s_config, 0, NULL);

    // Configure pins
    const i2s_pin_config_t pin_config = {
        .bck_io_num = sckPin,
        .ws_io_num = wsPin,
        .data_out_num = -1,
        .data_in_num = sdPin
    };

    i2s_set_pin(port, &pin_config);
    i2s_start(port);
}

float I2SMicrophone::readAverage(int rangelimit) {
    // Print false range to Serial Plotter
    Serial.print(rangelimit * -1);
    Serial.print(" ");
    Serial.print(rangelimit);
    Serial.print(" ");

    // Read data from I2S
    size_t bytesIn = 0;
    esp_err_t result = i2s_read(port, buffer, bufferLen, &bytesIn, portMAX_DELAY);

    if (result == ESP_OK) {
        int16_t samples_read = bytesIn / 2; // Each sample is 2 bytes
        if (samples_read > 0) {
            float mean = 0;
            for (int16_t i = 0; i < samples_read; ++i) {
                mean += buffer[i];
            }
            mean /= samples_read;
            Serial.println(mean);
            return mean;
        }
    }
    return 0.0;
}
