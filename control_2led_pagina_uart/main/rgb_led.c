/*
 * rgb_led.c
 *
 *  Created on: Oct 11, 2021
 *      Author: kjagu
 */

#include <stdbool.h>

#include "driver/ledc.h"
#include "rgb_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h" //Librería para configurar los timers
#include "driver/uart.h"    //Librería para puertos UART

static const char TAG[] = "rgbled";

// RGB LED Configuration Array
ledc_info_t ledc_ch[RGB_LED_CHANNEL_NUM];

// handle for rgb_led_pwm_init
bool g_pwm_init_handle = false;

// handle for rgb_led_init
bool g_LEDs_init_handle = false;

/**
 * Initializes the RGB LED settings per channel, including
 * the GPIO for each color, mode and timer configuration.
 */

QueueHandle_t ADC_lecture = 0;           // Se crea variable para almacenar cola de valores medidos por ADC
QueueHandle_t Set_values_queue = 0;      // Se crea variable para almacenar cola de valores leídos por el UART
QueueHandle_t Case_led_queue = 0;        // Se crea variable para almacenar cola de casos de los leds (case 1=B, case 2=G, case 3=R, case 0=NULL)
QueueHandle_t Potencia_led_queue = 0;    // Se crea variable para almacenar cola de la potencia que se le va a asignar a cada LED
QueueHandle_t Temperaturas = 0;          // Se crea variable para almacenar cola que contiene temperaturas tomadas en diferentes instantes de tiempo
Dict_set_values Values_set_min_max_LEDs; // Se crea variable con la estructura Dict_set_values
Dict_potencias_LEDs Potencias;           // Se crea variable con la estructura Dict_potencias_LEDs


esp_err_t init_uart(void) // Función para configurar e inicializar puerto UART
{
    uart_config_t uart_config = {
        .baud_rate = 115200, // Velocidad de transmisión
        .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .parity = UART_PARITY_DISABLE,
        .source_clk = UART_SCLK_APB,
        .stop_bits = UART_STOP_BITS_1};

    uart_param_config(UART_NUM, &uart_config);

    uart_set_pin(UART_NUM, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); // Se aquí se indica los pines a utilizar en Tx y Rx

    uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, size_queue_UART, &uart_queue, 0); // Se indica el tamaño de memoria para Tx y Rx. Además se crea la cola para los eventos del UART

    ESP_LOGI(tag_UART, "Init uart completed");

    return ESP_OK;
}

esp_err_t create_task(void) // Función en donde se crean y configuran las tareas
{
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    xTaskCreate(uart_task,
                "uart_task",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                5,
                &xHandle);

    ESP_LOGI(tag_task, "Tasks created");

    return ESP_OK;
}

void uart_task(void *pvParameters) /*Tarea para detectar información del UART, leerla,
                                    procesarla para ver si cumple con ciertas caracteristicas
                                     y finalmente almacenarla en una cola*/
{
    uart_event_t event;                                                                                                           // Variables para almacenar estructura del evento detectado
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);                                                                                  // Variable para almacenar información recibida
    extern QueueHandle_t uart_queue;                                                                                              // Se importa la cola de un archivo externo ya que la vamos a utilizar en este
    char *Mensaje_guia = "¡ESTRUCTURA DE DATOS INCORRECTA!\nComandos permitidos:\n LED_X=NUM\nEn donde: X->R,G o B\tNUM->Número"; // Mensaje par enviar por UART

    while (1)
    {
        if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY))
        {
            gpio_set_level(ledUART, 1);
            bzero(data, BUF_SIZE);

            switch (event.type)
            {
            case UART_DATA:                                                      // Si el tipo de evento es "UART_DATA", es decir, si llegó información, se ejecuta lo siguiente
                uart_read_bytes(UART_NUM, data, event.size, pdMS_TO_TICKS(500)); // Lee la información y la guarda en la variable "data"
                uart_write_bytes(UART_NUM, (const char *)data, event.size);      // Retorna lo mismo que llegó, no es necesario, solamentes es de verificación
                uart_flush(UART_NUM);                                            // Limpia el puerto por si queda algo. Esto para no tener problema en la próxima lectura

                char *ptr = strrchr((const char *)data, '='); // Revisa si "data" tiene un "=". Devuelve NULL si no está, de lo contrario devuleve la posición de la última vez que se presentó

                if (ptr != NULL) // Sí si está el caracter "="
                {
                    char *data_whitout_LB = strtok((const char *)data, "\n"); // Quita el salto de línea que enviar el terminal por defecto
                    // printf("data_whitout_LB= %s\n", data_whitout_LB);

                    char *data_key = strtok(data_whitout_LB, "="); // Separa la parte antes del igual (comando para identificar a qué LED se le hace el cambio)
                    Values_set_min_max_LEDs.key = data_key;        // Lo almacena en la estructura creada, exactamente en la varible "key" que contiene internamente
                    // printf("Key= %s\n", Values_set_min_max_LEDs.key);

                    char *data_value = strtok(NULL, "=");           // Separa la parte después del igual (Número a asignar)
                    int data_value_int = atoi(data_value);          // Converte el string en un entero
                    Values_set_min_max_LEDs.value = data_value_int; // Lo almacena en la estructura creada, exactamente en la varible "value" que contiene internamente
                    // printf("Value= %d\n", Values_set_min_max_LEDs.value);

                    // Guarda la estructura en la cola y en caso tal que no pueda, informa por medio de consola
                    !xQueueSend(Set_values_queue, &Values_set_min_max_LEDs, pdMS_TO_TICKS(100)) ? printf("Error cargando valor a la cola\n") : NULL;
                }
                else
                {
                    uart_write_bytes(UART_NUM_2, Mensaje_guia, strlen(Mensaje_guia)); // Envía mensaje de alerta a la terminar
                }

                break;

            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        gpio_set_level(ledUART, 0);
    }
}

static void rgb_led_pwm_init(void)
{
	int rgb_ch;

	// Red_A
	ledc_ch[0].channel = LEDC_CHANNEL_0;
	ledc_ch[0].gpio = RGB_LED_RED_GPIO;
	ledc_ch[0].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[0].timer_index = LEDC_TIMER_0;

	// Green_A
	ledc_ch[1].channel = LEDC_CHANNEL_1;
	ledc_ch[1].gpio = RGB_LED_GREEN_GPIO;
	ledc_ch[1].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[1].timer_index = LEDC_TIMER_0;

	// Blue_A
	ledc_ch[2].channel = LEDC_CHANNEL_2;
	ledc_ch[2].gpio = RGB_LED_BLUE_GPIO;
	ledc_ch[2].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[2].timer_index = LEDC_TIMER_0;

	// Red_B
	ledc_ch[3].channel = LEDC_CHANNEL_3;
	ledc_ch[3].gpio = RGB_LED_RED_GPIO_B;
	ledc_ch[3].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[3].timer_index = LEDC_TIMER_0;

	// Green_B
	ledc_ch[4].channel = LEDC_CHANNEL_4;
	ledc_ch[4].gpio = RGB_LED_GREEN_GPIO_B;
	ledc_ch[4].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[4].timer_index = LEDC_TIMER_0;

	// Blue_B
	ledc_ch[5].channel = LEDC_CHANNEL_5;
	ledc_ch[5].gpio = RGB_LED_BLUE_GPIO_B;
	ledc_ch[5].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[5].timer_index = LEDC_TIMER_0;

	// Configure timer zero
	ledc_timer_config_t ledc_timer =
		{
			.duty_resolution = LEDC_TIMER_8_BIT,
			.freq_hz = 100,
			.speed_mode = LEDC_HIGH_SPEED_MODE,
			.timer_num = LEDC_TIMER_0};
	ledc_timer_config(&ledc_timer);

	// Configure channels
	for (rgb_ch = 0; rgb_ch < RGB_LED_CHANNEL_NUM; rgb_ch++)
	{
		ledc_channel_config_t ledc_channel =
			{
				.channel = ledc_ch[rgb_ch].channel,
				.duty = 0,
				.hpoint = 0,
				.gpio_num = ledc_ch[rgb_ch].gpio,
				.intr_type = LEDC_INTR_DISABLE,
				.speed_mode = ledc_ch[rgb_ch].mode,
				.timer_sel = ledc_ch[rgb_ch].timer_index,
			};
		ledc_channel_config(&ledc_channel);
	}
	g_pwm_init_handle = true;
}

/**
 * Sets the RGB color.
 */
static void rgb_led_set_pwm_color(int row, uint8_t red, uint8_t green, uint8_t blue)
{
	if (row == 1)
	{
		// Value should be 0 - 255 for 8 bit number
		ledc_set_duty(ledc_ch[0].mode, ledc_ch[0].channel, red);
		ledc_update_duty(ledc_ch[0].mode, ledc_ch[0].channel);

		ledc_set_duty(ledc_ch[1].mode, ledc_ch[1].channel, green);
		ledc_update_duty(ledc_ch[1].mode, ledc_ch[1].channel);

		ledc_set_duty(ledc_ch[2].mode, ledc_ch[2].channel, blue);
		ledc_update_duty(ledc_ch[2].mode, ledc_ch[2].channel);
	}
	if (row == 2)
	{
		// Value should be 0 - 255 for 8 bit number
		ledc_set_duty(ledc_ch[3].mode, ledc_ch[3].channel, red);
		ledc_update_duty(ledc_ch[3].mode, ledc_ch[3].channel);

		ledc_set_duty(ledc_ch[4].mode, ledc_ch[4].channel, green);
		ledc_update_duty(ledc_ch[4].mode, ledc_ch[4].channel);

		ledc_set_duty(ledc_ch[5].mode, ledc_ch[5].channel, blue);
		ledc_update_duty(ledc_ch[5].mode, ledc_ch[5].channel);
	}
}

static void rgb_led_init(void)
{
	gpio_reset_pin(RGB_LED_RED_GPIO_SERVER);
	gpio_reset_pin(RGB_LED_GREEN_GPIO_SERVER);
	gpio_reset_pin(RGB_LED_BLUE_GPIO_SERVER);

	gpio_set_direction(RGB_LED_RED_GPIO_SERVER, GPIO_MODE_OUTPUT);
	gpio_set_direction(RGB_LED_GREEN_GPIO_SERVER, GPIO_MODE_OUTPUT);
	gpio_set_direction(RGB_LED_BLUE_GPIO_SERVER, GPIO_MODE_OUTPUT);

	g_LEDs_init_handle = true;
}

static void toggleLed(int GPIO)
{
	bool state = 0;
	for (int i = 0; i < 4; i++)
	{
		state = !state;
		gpio_set_level(GPIO, state);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

static void setStateLed(int R, int G, int B)
{
	// gpio_set_level(RGB_LED_RED_GPIO_SERVER, R);
	// gpio_set_level(RGB_LED_GREEN_GPIO_SERVER, G);
	// gpio_set_level(RGB_LED_BLUE_GPIO_SERVER, B);

	(R == 1) ? toggleLed(RGB_LED_RED_GPIO_SERVER) : NULL;
	(G == 1) ? toggleLed(RGB_LED_GREEN_GPIO_SERVER) : NULL;
	(B == 1) ? toggleLed(RGB_LED_BLUE_GPIO_SERVER) : NULL;
}

void rgb_led_wifi_app_started(void)
{
	(g_LEDs_init_handle == false) ? rgb_led_init() : NULL;
	setStateLed(1, 0, 0);
}

void rgb_led_http_server_started(void)
{
	(g_LEDs_init_handle == false) ? rgb_led_init() : NULL;
	setStateLed(0, 0, 1);
}

void rgb_led_wifi_connected(void)
{
	(g_LEDs_init_handle == false) ? rgb_led_init() : NULL;
	setStateLed(0, 1, 0);
}

void updateRGB(int row, int R, int G, int B)
{
	(g_pwm_init_handle == false) ? rgb_led_pwm_init() : NULL;
	int RValue = (R * 255) / 100;
	int GValue = (G * 255) / 100;
	int BValue = (B * 255) / 100;
	rgb_led_set_pwm_color(row, RValue, GValue, BValue);
}
