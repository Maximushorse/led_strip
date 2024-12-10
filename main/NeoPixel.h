#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <cstdint>
#include <vector>
#include <driver/rmt_tx.h>

class NeoPixel {
public:
    NeoPixel(uint16_t numPixels, rmt_channel_handle_t rmtChannel);
    ~NeoPixel();

    void begin(gpio_num_t gpioPin);
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
    void show();
    void clear();

private:
    uint16_t numPixels;                   // Liczba diod
    rmt_channel_handle_t rmtChannel;     // Kanał RMT
    rmt_encoder_handle_t rmt_encoder;    // Enkoder RMT
    std::vector<uint8_t> pixels;         // Bufor kolorów GRB

    void sendRmt(const uint8_t* data, size_t length);
};

#endif // NEOPIXEL_H
