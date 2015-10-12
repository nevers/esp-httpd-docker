#include "log.h"
#include "http.h"
#include "wifi.h"
#include "user_config.h"
#include "osapi.h"

void handle_system_init_done(void);
void handle_adc(HttpRequest* request, struct espconn* connection);

void ICACHE_FLASH_ATTR user_init() {
    log_init(INFO);
    system_init_done_cb(&handle_system_init_done);

}

void handle_system_init_done() {
    wifi_init();
    http_init();
    http_add_request_handler(&handle_adc);
}

void handle_adc(HttpRequest* request, struct espconn* connection) {
    uint8_t buffer[16];
    uint16 value = system_adc_read();
    os_sprintf(buffer, "[adc] %d\n", value);
    http_send(connection, buffer);
}
