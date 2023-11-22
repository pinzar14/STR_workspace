/*
 * rgb_led.h
 *
 *  Created on: Oct 11, 2021
 *      Author: kjagu
 */

#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

// RGB LED GPIOs
#define RGB_LED_RED_GPIO		5
#define RGB_LED_GREEN_GPIO		18
#define RGB_LED_BLUE_GPIO		19

#define RGB_LED_RED_GPIO_B		21
#define RGB_LED_GREEN_GPIO_B		22
#define RGB_LED_BLUE_GPIO_B		23

#define RGB_LED_RED_GPIO_SERVER		33
#define RGB_LED_GREEN_GPIO_SERVER		25
#define RGB_LED_BLUE_GPIO_SERVER		26

// RGB LED color mix channels
#define RGB_LED_CHANNEL_NUM		6

// RGB LED configuration
typedef struct
{
	int channel;
	int gpio;
	int mode;
	int timer_index;
} ledc_info_t;
// ledc_info_t ledc_ch[RGB_LED_CHANNEL_NUM]; Move this declaration to the top of rgb_led.c to avoid linker errors

#define NTC_PIN ADC_CHANNEL_7 // Canal ocupado para la lectura ADC
#define UART_NUM UART_NUM_2   // Puerto UART utilizado
#define UART_TX 17            // GPIO pin TX UART
#define UART_RX 16            // GPIO pin RX UART
#define size_queue_UART 5     // Tamaño de cola para almacenar envento UART
#define BUF_SIZE 1024         // Tamaño de memoria para datos de entrada y salida
#define ledUART 2             // GPIO LED indicador de lectura UART (led amarillo)

/**
 * Color to indicate WiFi application has started.
 */
void rgb_led_wifi_app_started(void);

/**
 * Color to indicate HTTP server has started.
 */
void rgb_led_http_server_started(void);

/**
 * Color to indicate that the ESP32 is connected to an access point.
 */
void rgb_led_wifi_connected(void);

esp_err_t create_task(void);
void uart_task(void *pvParameters);
void updateRGB(int row, int R, int G, int B);
esp_err_t init_uart(void);

typedef struct
{
    char *key;
    int ledRed;
    int ledGreen;
    int ledBlue;
} Dict_potencias_LEDs;

typedef struct
{
    char *key;
    int value;
} Dict_set_values; // Estructua para manejar de una mejor forma la información que llega por el UART


#endif /* MAIN_RGB_LED_H_ */
