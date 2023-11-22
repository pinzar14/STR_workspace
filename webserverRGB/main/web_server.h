#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_err.h"



esp_err_t root_get_handler(httpd_req_t *req);
httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);
void disconnect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);


/*esp_err_t init_led(void);
esp_err_t set_color(uint32_t red, uint32_t green, uint32_t blue);*/

#endif