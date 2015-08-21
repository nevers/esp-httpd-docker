#include "wifi.h"
#include "log.h"
#include "user_interface.h"
#include "osapi.h"

void wifi_callback(System_Event_t *event);

void wifi_init() {
    println("[wifi] init");
    char ssid[32] = WIFI_SSID;
    char pwd[64] = WIFI_PWD;
    struct station_config config;

    wifi_set_opmode(STATION_MODE);
    wifi_station_set_hostname(WIFI_HOST);

    os_memcpy(&config.ssid, ssid, 32);
    os_memcpy(&config.password, pwd, 64);

    wifi_station_set_config(&config);
    wifi_set_event_handler_cb(wifi_callback);
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
