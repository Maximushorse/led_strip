#include "NeoPixel.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"

#define NUM_PIXELS 8
#define GPIO_PIN GPIO_NUM_18

extern "C" void app_main() {
    NeoPixel strip(NUM_PIXELS, RMT_CHANNEL_0);
    strip.begin(GPIO_PIN);

    // Ustaw kolory pikseli
    strip.setPixelColor(0, 255, 0, 0); // Czerwony
    strip.setPixelColor(1, 0, 255, 0); // Zielony
    strip.setPixelColor(2, 0, 0, 255); // Niebieski
    strip.show();

    vTaskDelay(pdMS_TO_TICKS(1000));

    strip.clear();
}













 /*
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "led_strip_encoder.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      1

#define EXAMPLE_LED_NUMBERS         3
#define EXAMPLE_CHASE_SPEED_MS      100

static const char *TAG = "example";

static uint8_t led_strip_pixels[EXAMPLE_LED_NUMBERS * 3];


void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}

void app_main(void)
{
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint16_t hue = 0;
    uint16_t start_rgb = 0;

    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_channel_handle_t led_chan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");
    rmt_encoder_handle_t led_encoder = NULL;
    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    ESP_LOGI(TAG, "Start LED rainbow chase");
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };
    while (1) {

                // Build RGB pixels
                hue = 1 * 360 / EXAMPLE_LED_NUMBERS + start_rgb;
                led_strip_hsv2rgb(hue, 10, 10, &red, &green, &blue);
                led_strip_pixels[1] = green;
                led_strip_pixels[2] = blue;
                led_strip_pixels[30] = red;

            // Flush RGB values to LEDs
            ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
            ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
            memset(led_strip_pixels, 0, sizeof(led_strip_pixels));
            ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
            ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
        
        start_rgb += 60;
    }
}




#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "esp_log.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0 // Kanał RMT
#define LED_GPIO_PIN 0               // GPIO sterujące
#define LED_COUNT 30                 // Liczba diod
#define MAX_BRIGHTNESS 255           // Maksymalna jasność diody

// Konwersja koloru RGB na format wymagany przez WS2812B
static void set_led_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, rmt_item32_t* items) {
    uint8_t adjusted_r = (r * brightness) / 100;
    uint8_t adjusted_g = (g * brightness) / 100;
    uint8_t adjusted_b = (b * brightness) / 100;

    for (int led = 0; led < LED_COUNT; led++) {
        for (int i = 0; i < 24; i++) {
            uint8_t bit = (i < 8) ? adjusted_g : (i < 16) ? adjusted_r : adjusted_b;
            bit = bit & (1 << (7 - (i % 8)));

            if (bit) {
                items[led * 24 + i].level0 = 1;
                items[led * 24 + i].duration0 = 9; // Długość logicznego "1"
                items[led * 24 + i].level1 = 0;
                items[led * 24 + i].duration1 = 3;
            } else {
                items[led * 24 + i].level0 = 1;
                items[led * 24 + i].duration0 = 3; // Długość logicznego "0"
                items[led * 24 + i].level1 = 0;
                items[led * 24 + i].duration1 = 9;
            }
        }
    }

    // Dodaj zakończenie sygnału
    items[LED_COUNT * 24].level0 = 0;
    items[LED_COUNT * 24].duration0 = 0;
    items[LED_COUNT * 24].level1 = 0;
    items[LED_COUNT * 24].duration1 = 0;
}

static void send_colors(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    rmt_item32_t items[(LED_COUNT * 24) + 1];
    set_led_color(r, g, b, brightness, items);

    // Wyślij dane
    ESP_ERROR_CHECK(rmt_write_items(RMT_TX_CHANNEL, items, (LED_COUNT * 24) + 1, true));
    rmt_wait_tx_done(RMT_TX_CHANNEL, portMAX_DELAY);
}

void app_main(void) {
    // Konfiguracja RMT
    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = RMT_TX_CHANNEL,
        .gpio_num = LED_GPIO_PIN,
        .clk_div = 80, // Dzielnik zegara, przy 80 MHz = 1 MHz (1 µs na tick)
        .mem_block_num = 1,
        .tx_config = {
            .loop_en = false,
            .carrier_en = false,
            .idle_output_en = true,
            .idle_level = RMT_IDLE_LEVEL_LOW,
        },
    };
    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(RMT_TX_CHANNEL, 0, 0));

    // Główna pętla
    while (true) {
        // 1. Świecenie na czerwono 10% przez 4 sekundy
        send_colors(255, 0, 0, 10);
        vTaskDelay(pdMS_TO_TICKS(4000));

        // 2. Miganie, zwiększając jasność do 30% co sekundę (5x)
        for (int brightness = 10; brightness <= 30; brightness += 5) {
            send_colors(255, 0, 0, brightness);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        // 3. Zapalanie kolejnych diod na pomarańczowo 10% (5x)
        for (int cycle = 0; cycle < 5; cycle++) {
            for (int i = 0; i < LED_COUNT; i++) {
                send_colors(255, 100, 0, 10); // Pomarańczowy kolor
                vTaskDelay(pdMS_TO_TICKS(33)); // Krótka pauza dla efektu
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); // Pauza po cyklu
        }
    }
}

*/
