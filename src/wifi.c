#include "wifi.h"
#include "log.h"
#include "user_interface.h"
#include "osapi.h"

void wifi_callback(System_Event_t *event);

void wifi_init() {
    static struct station_config config;

    logln_info("[wifi] init");

    wifi_set_event_handler_cb(wifi_callback);
    wifi_station_set_auto_connect(0);
    wifi_station_set_hostname(WIFI_HOST);
    wifi_set_opmode_current(STATION_MODE);

    config.bssid_set = 0;
    os_memcpy(&config.ssid, WIFI_SSID, 32);
    os_memcpy(&config.password, WIFI_PWD, 64);

    wifi_station_set_config_current(&config);
    wifi_station_connect();
}

void wifi_callback(System_Event_t *evt) {
    switch(evt->event) {
        case EVENT_STAMODE_CONNECTED:
            logln_info("[wifi] connected");
            break;

         case EVENT_STAMODE_DISCONNECTED:
            logln_info("[wifi] disconnected");
            break;

         case EVENT_STAMODE_GOT_IP:
            log_info("[wifi] got ip: ");
            char ip[16];
            os_sprintf(ip, IPSTR, IP2STR(&evt->event_info.got_ip.ip));
            logln_info(ip);
            break;
    }
}
