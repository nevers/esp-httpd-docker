#include "log.h"
#include "http.h"
#include "user_config.h"
#include "osapi.h"

void wifi_callback(System_Event_t *event);
void handle_adc(HttpRequest* request, struct espconn* connection);

void ICACHE_FLASH_ATTR user_init() {
    log_init();
    println("[wifi] init");
    char ssid[32] = SSID;
    char pwd[64] = PWD;
    struct station_config config;

    wifi_set_opmode(STATION_MODE);
    wifi_station_set_hostname(HOST);

    os_memcpy(&config.ssid, ssid, 32);
    os_memcpy(&config.password, pwd, 64);

    wifi_station_set_config(&config);
    wifi_set_event_handler_cb(wifi_callback);

    http_init();
    http_add_request_handler(&handle_adc);
}

void handle_adc(HttpRequest* request, struct espconn* connection) {
    uint8_t buffer[16];
    uint16 value = system_adc_read();
    os_sprintf(buffer, "[adcr] %d\n", value);
    http_send(connection, buffer);
}

void wifi_callback(System_Event_t *evt) {
    switch(evt->event) {
        case EVENT_STAMODE_CONNECTED:
            println("[wifi] connected");
            break;

         case EVENT_STAMODE_DISCONNECTED:
            println("[wifi] disconnected");
            break;

         case EVENT_STAMODE_GOT_IP:
            print("[wifi] got ip: ");
            char ip[16];
            os_sprintf(ip, IPSTR, IP2STR(&evt->event_info.got_ip.ip));
            println(ip);
            break;
    }
}
