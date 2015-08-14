#include "user_config.h"
#include "user_interface.h"
#include "gpio.h"
#include "osapi.h"

void ICACHE_FLASH_ATTR user_init() {
    gpio_init();

    os_printf("Initialising wifi\n");
    char ssid[32] = SSID;
    char pwd[64] = PWD;
    struct station_config config;

    wifi_set_opmode(STATION_MODE);
    wifi_station_set_hostname(HOST);

    os_memcpy(&config.ssid, ssid, 32);
    os_memcpy(&config.password, pwd, 64);

    os_printf("Setting wifi configuration\n");
    wifi_station_set_config(&config);
}
