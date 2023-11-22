#ifndef CONFIG_NTC
#define CONFIG_NTC

#include <stdio.h>
#include "driver/adc.h" //Librería para configurar y leer por medio de ADC
#include "esp_log.h"    //librería para poder imprimir texto con colores diferentes
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h" //Librería para las colas
#include <stdlib.h>
#include <math.h>       //Librería para hacer operaciones matemáticas. En este caso se utilizo para el logaritmo natural
#include <string.h>     //Librería para manejo de cadenas de caracteres
#include "freertos/task.h"  //Librería para las Tareas

#define NTC_PIN ADC_CHANNEL_7 // Canal ocupado para la lectura ADC
#define R_FIXED 100000            // Valor de resistencia adicional que se pone para hacer el valor de tensión con la NTC
#define R0_NTC 100000             // Valor de NTC a 25°C
#define Beta 4190                 // Factor Beta de la NTC
#define Temp0 298.15              // Valor de temperatura a temperatura ambiente, en °Kelvin. (25°C+273.15=298.15°K)
#define Vol_REF 3.3               // Voltaje aplicado al divisor de tensión
#define SIZE_BUFFER_TASK 1024 * 2 // valor de espacio de memoria para las tareas (si se pone un valor muy pequeño se va a reiniciar el uC)
#define Delay_promedio 500

static const char *tag_task = "Task"; // Variable utilzada para etiquetar con la palabra "Task" el mensaje enviado por medio de ESP_LOG()

// Prototipado de funciones
void inicializeNTC(void);
esp_err_t create_task_NTC(void);
void get_ADC(void *pvParameters);
void Promedio_temp(void *pvParameters);

static const char *tag_ADC = "ADC"; // Variable utilzada para etiquetar con la palabra "ADC" el mensaje enviado por medio de ESP_LOG()

// Prototipado de funciones
esp_err_t set_adc(void);

#endif