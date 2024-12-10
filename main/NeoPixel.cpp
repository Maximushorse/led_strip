#include "NeoPixel.h"
#include <esp_log.h>
#include <cstring>

#define TAG "NeoPixel"

#define WS2812_T0H 400  // 0: wysoki poziom 400 ns
#define WS2812_T1H 800  // 1: wysoki poziom 800 ns
#define WS2812_T0L 850  // 0: niski poziom 850 ns
#define WS2812_T1L 450  // 1: niski poziom 450 ns
#define WS2812_RESET 50000 // Reset - niski poziom przez 50 µs

NeoPixel::NeoPixel(uint16_t numPixels, rmt_channel_handle_t rmtChannel)
    : numPixels(numPixels), rmtChannel(rmtChannel) {
    pixels.resize(numPixels * 3, 0); // Inicjalizacja wszystkich pikseli na czarno
    rmt_encoder = nullptr;          // Domyślnie brak enkodera
}

NeoPixel::~NeoPixel() {
    if (rmt_encoder) {
        rmt_del_encoder(rmt_encoder);
    }
    rmt_del_channel(rmtChannel);
}

void NeoPixel::begin(gpio_num_t gpioPin) {
    rmt_tx_channel_config_t config = {
        .gpio_num = gpioPin,
        .clk_src = RMT_CLK_SRC_APB,
        .resolution_hz = 10'000'000, // 10 MHz
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&config, &rmtChannel));
    ESP_ERROR_CHECK(rmt_enable(rmtChannel));

    rmt_copy_encoder_config_t encoder_config = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&encoder_config, &rmt_encoder));
}

void NeoPixel::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    if (n < numPixels) {
        pixels[n * 3] = g;
        pixels[n * 3 + 1] = r;
        pixels[n * 3 + 2] = b;
    }
}

void NeoPixel::show() {
    sendRmt(pixels.data(), pixels.size());
}

void NeoPixel::clear() {
    std::fill(pixels.begin(), pixels.end(), 0);
    show();
}

void NeoPixel::sendRmt(const uint8_t* data, size_t length) {
    rmt_transmit_config_t tx_config = {.loop_count = 0};
    ESP_ERROR_CHECK(rmt_transmit(rmtChannel, rmt_encoder, data, length, &tx_config));
}
