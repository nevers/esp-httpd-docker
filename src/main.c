#include "log.h"
#include "http.h"
#include "wifi.h"
#include "user_config.h"
#include "osapi.h"

void handle_adc(HttpRequest* request, struct espconn* connection);

void ICACHE_FLASH_ATTR user_init() {
    log_init();
    wifi_init();
    http_init();
    http_add_request_handler(&handle_adc);
}

void handle_adc(HttpRequest* request, struct espconn* connection) {
    uint8_t buffer[16];
    uint16 value = system_adc_read();
    os_sprintf(buffer, "[adcr] %d\n", value);
    http_send(connection, buffer);
}

