#include "user_config.h"
#include "user_interface.h"
#include "eagle_soc.h"
#include "osapi.h"
#include "uart_register.h"

void disableDebugMessages();
void nop(char);
void wifi_callback(System_Event_t *event);
void println(uint8_t* message);
void print(uint8_t* message);
void print_char(uint8_t character);

void ICACHE_FLASH_ATTR user_init() {
    disableDebugMessages();
    uart_div_modify(0, UART_CLK_FREQ/115200); // Set the UART baud rate
    print("\033[2J"); // Clear the screen
    println("\n\r***");
    println("Initialising wifi");
    char ssid[32] = SSID;
    char pwd[64] = PWD;
    struct station_config config;

    wifi_set_opmode(STATION_MODE);
    wifi_station_set_hostname(HOST);

    os_memcpy(&config.ssid, ssid, 32);
    os_memcpy(&config.password, pwd, 64);

    println("Setting wifi configuration");
    wifi_station_set_config(&config);
    wifi_set_event_handler_cb(wifi_callback);
}

void disableDebugMessages() {
    os_install_putc1(nop);    
}

void nop(char c) {
}

void println(uint8_t* message) {
    int length = strlen(message);
    message[length] = '\n';
    message[length + 1] = '\r';
    message[length + 2] = '\0';
    print(message);
}

void print(uint8_t* message) {
    int i;
    for(i = 0; i < strlen(message); i++)
        print_char(message[i]);
}

void print_char(uint8_t character) {
    while (true) {
        uint32_t fifo_cnt = READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126)
            break;
    }
    WRITE_PERI_REG(UART_FIFO(0) , character);
}

void wifi_callback(System_Event_t *event) {
    println("Got wifi callback");
    switch(event->event) {
        case EVENT_STAMODE_CONNECTED:
            println("Connected!");
            break;

         case EVENT_STAMODE_DISCONNECTED:
            println("Disconnected!");
            break;

         case EVENT_STAMODE_GOT_IP:
            println("Got ip!");
            //char ip[20];
            //os_sprintf(ip, "%d", IP2STR(&event->event_info.got_ip.ip));
            //println(ip);
            break;
    }
}
