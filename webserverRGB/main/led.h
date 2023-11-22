#ifndef LED_H
#define LED_H

#include "esp_err.h"
//nuevas
esp_err_t set_color2(uint32_t red, uint32_t green, uint32_t blue);
esp_err_t toggle_led2(int led2);

esp_err_t init_led(void);
esp_err_t set_color(uint32_t red, uint32_t green, uint32_t blue);
esp_err_t toggle_led(int led);
extern int8_t led_g_state;
extern int8_t led_r_state;
extern int8_t led_b_state;

extern int8_t led_g2_state;
extern int8_t led_r2_state;
extern int8_t led_b2_state;


#endif