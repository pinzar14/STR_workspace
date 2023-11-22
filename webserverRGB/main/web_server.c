#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"
#include <esp_https_server.h>
#include "esp_tls.h"
#include <string.h>
#include "driver/gpio.h"
#include <stdio.h>

#include "led.h"
#include "web_server.h"

static const char *TAG = "WEBSERVER"; 

#define ledR 27
#define ledG 26
#define ledB 25

#define led2R 32
#define led2G 33
#define led2B 35

/*gpio_num_t ledG;
gpio_num_t ledR;
gpio_num_t ledB;*/

/* 
// Declaraciones para las variables de estado de los LEDs (asumiendo int)
int led_g_state = 0;
int led_r_state = 0;
int led_b_state = 0;
 */

esp_err_t root_get_handler(httpd_req_t *req)
{
    extern unsigned char view_start[] asm("_binary_view_html_start");
    extern unsigned char view_end[] asm("_binary_view_html_end");
    size_t view_len = view_end - view_start;
    char viewHtml[view_len];
    memcpy(viewHtml, view_start, view_len);
    ESP_LOGI(TAG, "URI: %s", req->uri);

    if (strcmp(req->uri, "/?led-r") == 0)
    {
        toggle_led(ledR);
    }
    if (strcmp(req->uri, "/?led-g") == 0)
    {
        toggle_led(ledG);
    }
    if (strcmp(req->uri, "/?led-b") == 0)
    {
        toggle_led(ledB);
    }

    //nuevo parcial
    if (strcmp(req->uri, "/?led2-r2") == 0)
    {
        toggle_led2(led2R);
    }
    if (strcmp(req->uri, "/?led2-g2") == 0)
    {
        toggle_led(led2G);
    }
    if (strcmp(req->uri, "/?led2-b2") == 0)
    {
        toggle_led(led2B);
    }


    char *viewHtmlUpdated;
    int formattedStrResult = asprintf(&viewHtmlUpdated, viewHtml, led_r_state ? "ON" : "OFF", led_g_state ? "ON" : "OFF", led_b_state ? "ON" : "OFF");

    httpd_resp_set_type(req, "text/html");

    if (formattedStrResult > 0)
    {
        httpd_resp_send(req, viewHtmlUpdated, view_len);
        free(viewHtmlUpdated);
    }
    else
    {
        ESP_LOGE(TAG, "Error updating variables");
        httpd_resp_send(req, viewHtml, view_len);
    }

    return ESP_OK;
}

const httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server");

    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
    conf.transport_mode = HTTPD_SSL_TRANSPORT_INSECURE;
    esp_err_t ret = httpd_ssl_start(&server, &conf);
    if (ESP_OK != ret)
    {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }

    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &root);
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_ssl_stop(server);
}

void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        stop_webserver(*server);
        *server = NULL;
    }
}

void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        *server = start_webserver();
    }
}