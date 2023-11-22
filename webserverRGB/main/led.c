#include <stdio.h>
#include "driver/gpio.h"
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

#define ledR 27
#define ledG 26
#define ledB 25

#define led2R 32
#define led2G 33
#define led2B 35

int8_t led_r_state = 0;
int8_t led_g_state = 0;
int8_t led_b_state = 0;

int8_t led_r2_state = 0;
int8_t led_g2_state = 0;
int8_t led_b2_state = 0;



static const char *TAG = "led";

esp_err_t init_led(void) {

    gpio_config_t pGPIOConfig;
    pGPIOConfig.pin_bit_mask = (1ULL << ledR) | (1ULL << ledG) | (1ULL << ledB) | (1ULL << led2R) | (1ULL << led2G) | (1ULL << led2B);
    pGPIOConfig.mode = GPIO_MODE_DEF_OUTPUT;
    pGPIOConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    pGPIOConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pGPIOConfig.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&pGPIOConfig);

    ESP_LOGI(TAG, "init led completed");
    return ESP_OK;
}

esp_err_t set_color(uint32_t red, uint32_t green, uint32_t blue) {

    gpio_set_level(ledR, red);
    gpio_set_level(ledG, green);
    gpio_set_level(ledB, blue);

    return ESP_OK;
}

//nueva
esp_err_t set_color2(uint32_t red, uint32_t green, uint32_t blue) {

    gpio_set_level(led2R, red);
    gpio_set_level(led2G, green);
    gpio_set_level(led2B, blue);

    return ESP_OK;
}

esp_err_t toggle_led(int led){

    switch (led)
    {
    case ledR:
        led_r_state = !led_r_state;
        break;
    case ledG:
        led_g_state = !led_g_state;
        break;
    case ledB:
        led_b_state = !led_b_state;
        break;

    default:
        set_color(0,0,0);
        break;
    }
    set_color(led_r_state,led_g_state,led_b_state);
    return ESP_OK;
}
//nuevo
esp_err_t toggle_led2(int led2){

    switch (led2)
    {
    case led2R:
        led_r2_state = !led_r2_state;
        break;
    case led2G:
        led_g2_state = !led_g2_state;
        break;
    case led2B:
        led_b2_state = !led_b2_state;
        break;

    default:
        set_color2(0,0,0);
        break;
    }
    set_color2(led_r2_state,led_g2_state,led_b2_state);
    return ESP_OK;
}
